#include <cmath>
#include <cstdint>
#include <iostream>
#include <spdlog/spdlog.h>
#include <stddef.h>
#include <vector>

bool gen_square_wave(float freq_mhz, float sample_rate_msps,
                     float sample_time_s, float high_val, float low_val,
                     std::vector<float>& wave //
) {
  if (2 * freq_mhz > sample_rate_msps) {
    SPDLOG_ERROR("Sample rate under the nyquist limit");
    return false;
  }

  const uint32_t samples_per_period = sample_rate_msps / freq_mhz;
  const size_t n_samples = 1024;

  wave.clear();
  wave.reserve(n_samples);

  SPDLOG_DEBUG("Generate square wave. [n_samples={}, samples_pp = {}]",
               n_samples, samples_per_period);

  for (size_t i = 0; i < n_samples; ++i) {
    if (i % samples_per_period >= samples_per_period / 2) {
      wave.push_back(high_val);
    } else {
      wave.push_back(low_val);
    }
  }
  return true;
};

bool gen_sin_wave(float freq_mhz, float sample_rate_msps, float sample_time_s,
                  float high_val, float low_val, std::vector<float> &output //
) {
  if (sample_rate_msps / freq_mhz < 2) {
    SPDLOG_ERROR("Sample rate under the nyquist limit");
    return false;
  }

  const uint32_t samples_per_period = sample_rate_msps / freq_mhz;
  const size_t n_samples = 1024;

  const double ampl = (high_val - low_val) / 2;
  output.clear();
  output.reserve(n_samples);

  SPDLOG_DEBUG("Generate square wave. [n_samples={}, samples_pp = {}]",
               n_samples, samples_per_period);

  for (size_t i = 0; i < n_samples; ++i) {
    double v = ampl * std::sin(2 * M_PI * freq_mhz * static_cast<double>(i) /
                               sample_rate_msps);
    output.emplace_back(v);
  }
  return true;
};
