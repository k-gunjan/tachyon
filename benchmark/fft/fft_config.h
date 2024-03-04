#ifndef BENCHMARK_FFT_FFT_CONFIG_H_
#define BENCHMARK_FFT_FFT_CONFIG_H_

#include <stdint.h>

#include <vector>

namespace tachyon {

class FFTConfig {
 public:
  FFTConfig() = default;
  FFTConfig(const FFTConfig& other) = delete;
  FFTConfig& operator=(const FFTConfig& other) = delete;

  const std::vector<uint64_t>& exponents() const { return exponents_; }
  bool run_ifft() const { return run_ifft_; }
  bool check_results() const { return check_results_; }

  bool Parse(int argc, char** argv);

  std::vector<uint64_t> GetDegrees() const;

 private:
  std::vector<uint64_t> exponents_;
  bool run_ifft_ = false;
  bool check_results_ = false;
};

}  // namespace tachyon

#endif  // BENCHMARK_FFT_FFT_CONFIG_H_
