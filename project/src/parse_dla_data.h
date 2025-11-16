#ifndef PARSE_DLA_DATA_H
#define PARSE_DLA_DATA_H

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

struct SLogicAnalogHeader {
  char identifier[8];
  int32_t version;
  int32_t type;
};

struct SLogicAnalogData {
  SLogicAnalogHeader header;
  double begin_time;
  uint64_t sample_rate;
  uint64_t downsample;
  uint64_t num_samples;
  std::vector<float> samples;
};

std::optional<SLogicAnalogData> parse_dla_file(const std::string &path);

#endif
