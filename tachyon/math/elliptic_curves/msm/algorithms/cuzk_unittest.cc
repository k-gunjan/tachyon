#include "tachyon/math/elliptic_curves/msm/algorithms/cuzk.h"

#include "gtest/gtest.h"

#include "tachyon/base/containers/container_util.h"
#include "tachyon/device/gpu/gpu_logging.h"
#include "tachyon/device/gpu/gpu_memory.h"
#include "tachyon/math/elliptic_curves/bn/bn254/g1_gpu.h"
#include "tachyon/math/elliptic_curves/msm/test/msm_test_set.h"
#include "tachyon/math/elliptic_curves/short_weierstrass/affine_point.h"
#include "tachyon/math/elliptic_curves/short_weierstrass/point_xyzz.h"

namespace tachyon::math {

namespace {

using namespace device;

class CUZKTest : public testing::Test {
 public:
  static void SetUpTestSuite() {
    GPU_MUST_SUCCESS(gpuDeviceReset(), "");
    bn254::G1PointXYZZ::Curve::Init();
  }
};

}  // namespace

TEST_F(CUZKTest, ReduceBuckets) {
  size_t size = 1 << 10;
  PippengerCtx ctx = PippengerCtx::CreateDefault<bn254::Fr>(size);
  size_t start_group = 0;
  size_t end_group = ctx.window_count;
  size_t window_length = ctx.GetWindowLength();
  std::vector<bn254::G1PointXYZZ> cpu_buckets =
      CreatePseudoRandomPoints<bn254::G1PointXYZZ>(window_length *
                                                   ctx.window_count);
  auto gpu_buckets = gpu::GpuMemory<bn254::G1PointXYZZGpu>::MallocManaged(
      window_length * ctx.window_count);
  auto gpu_result =
      device::gpu::GpuMemory<bn254::G1PointXYZZGpu>::MallocManaged(1);
  for (size_t i = 0; i < gpu_buckets.size(); ++i) {
    gpu_buckets[i] = ConvertPoint<bn254::G1PointXYZZGpu>(cpu_buckets[i]);
  }
  CUZK<bn254::G1AffinePointGpu::Curve> cuzk;
  cuzk.SetContextForTesting(ctx);
  cuzk.SetGroupsForTesting(start_group, end_group);
  cuzk.SetSizesForTesting(2, 2);
  EXPECT_TRUE(cuzk.ReduceBuckets(std::move(gpu_buckets), gpu_result));
  GPU_MUST_SUCCESS(gpuDeviceSynchronize(), "");

  std::vector<bn254::G1PointXYZZ> window_sums = base::CreateVector(
      ctx.window_count, [window_length, &cpu_buckets](size_t i) {
        return PippengerBase<bn254::G1AffinePoint>::AccumulateBuckets(
            absl::MakeConstSpan(&cpu_buckets[i * window_length + 1],
                                &cpu_buckets[(i + 1) * window_length]));
      });

  auto expected = PippengerBase<bn254::G1AffinePoint>::AccumulateWindowSums(
      window_sums, ctx.window_bits);
  EXPECT_EQ(ConvertPoint<bn254::G1PointXYZZ>(*gpu_result.get()), expected);
}

}  // namespace tachyon::math