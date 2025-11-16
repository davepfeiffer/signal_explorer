#ifndef SPI_ANALYSIS_H
#define SPI_ANALYSIS_H

#include <vector>

extern void fft_with_noise(const std::vector<float> &data, float noise_scale,
                           std::vector<float> &result);

#endif
