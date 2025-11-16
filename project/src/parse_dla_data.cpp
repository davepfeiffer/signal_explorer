#include "parse_dla_data.h"

#include <cstring>
#include <fstream>
#include <optional>
#include <spdlog/spdlog.h>

//
// Quick and dirty file parser for the SaleaeLogc 2 DLA. See:
//
// https://support.saleae.com/troubleshooting/technical-faq/binary-export-format-logic-2
//
std::optional<SLogicAnalogData> parse_dla_file(const std::string &path) {
  std::ifstream istream(path, std::ios::binary);
  if (!istream) {
    SPDLOG_ERROR("Failed to open file: " + path);
    return std::nullopt;
  }

  SLogicAnalogData data;

  static_assert(std::endian::native == std::endian::little,
                "Big endian systems aren't supported.");

  // validate the header
  istream.read(data.header.identifier, 8);
  if (std::strncmp(data.header.identifier, "<SALEAE>", 8) != 0) {
    SPDLOG_ERROR("Unrecognized format invalid identifier.");
    return std::nullopt;
  }
  istream.read(reinterpret_cast<char *>(&data.header.version), sizeof(int32_t));
  if (data.header.version != 0) {
    SPDLOG_ERROR("Unsupported version. [expected_version=0, found_version={}]",
                 data.header.version);
    return std::nullopt;
  }
  istream.read(reinterpret_cast<char *>(&data.header.type), sizeof(int32_t));
  if (data.header.type != 1) {
    SPDLOG_ERROR("Not an analog file type. [expected_type=1, found_type={}]",
                 data.header.type);
    return std::nullopt;
  }

  // read the actual data
  istream.read(reinterpret_cast<char *>(&data.begin_time), sizeof(double));
  istream.read(reinterpret_cast<char *>(&data.sample_rate), sizeof(uint64_t));
  istream.read(reinterpret_cast<char *>(&data.downsample), sizeof(uint64_t));
  istream.read(reinterpret_cast<char *>(&data.num_samples), sizeof(uint64_t));

  if (!istream) {
    SPDLOG_ERROR("Failed to read file fixed data");
    return std::nullopt;
  }

  data.samples.resize(data.num_samples);
  istream.read(reinterpret_cast<char *>(data.samples.data()),
               data.num_samples * sizeof(float));

  if (!istream) {
    SPDLOG_ERROR("Failed to read sample data");
    return std::nullopt;
  }

  return data;
}
