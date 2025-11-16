#ifndef GEN_SQUARE_WAVE_H
#define GEN_SQUARE_WAVE_H

#include <vector>

extern bool gen_square_wave(float freq_mhz, float sample_rate_msps,
                            float sample_time_ms, float high_val, float low_val,
                            std::vector<float> &output);

extern bool gen_sin_wave(float freq_mhz, float sample_rate_msps,
                         float sample_time_ms, float high_val, float low_val,
                         std::vector<float> &output);

#endif
