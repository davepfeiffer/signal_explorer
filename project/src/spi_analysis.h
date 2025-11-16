#ifndef SPI_ANALYSIS_H
#define SPI_ANALYSIS_H

#include <vector>

extern void process_signal(const std::vector<float> &data, float noise_scale,
                           std::vector<double> &result);

#endif
