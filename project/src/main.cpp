#include <argparse/argparse.hpp>
#include <iostream>
#include <spdlog/spdlog.h>

#include "gen_square_wave.h"
#include "parse_dla_data.h"
#include "spi_analysis.h"

int main(int argc, char *argv[]) {
  spdlog::set_level(spdlog::level::debug);
  argparse::ArgumentParser parser("square_wave_fft");

  parser.add_argument("blocks")
      .help("number of blocks to use")
      .scan<'i', int>();
  parser.add_argument("--square")
      .help("Generate a square wave to analyze with the given frequency in MHz")
      .scan<'f', float>();
  parser.add_argument("--sin")
      .help("Generate a sin wave to analyze with the given frequency in MHz")
      .scan<'f', float>();
  parser.add_argument("-n", "--noise")
      .help("Noise scale to use. (Gaussian from 0-1 will be multiplied by this "
            "and added to the signal).")
      .scan<'f', float>();
  parser.add_argument("-i", "--input")
      .nargs(argparse::nargs_pattern::any)
      .help("path(s) to the signal data to be processed")
      .action([](const std::string &value) { return value; });
  parser.add_argument("--verbose")
      .help("Enable verbose output")
      .implicit_value(true)
      .default_value(false);

  try {
    parser.parse_args(argc, argv);
  } catch (const std::exception &err) {
    std::cerr << err.what() << std::endl;
    std::cerr << parser;
    return 1;
  }

  // disable logging so stdout can be piped to a CSV easier
  if (parser.get<bool>("--verbose") == false) {
    spdlog::set_level(spdlog::level::off);
  }

  auto blocks = parser.get<int>("blocks");

  if (blocks == 0) {
    std::cout << "Zero blocks. [n_threads="
              << ", n_blocks=" << blocks << "\n";
  }

  float noise = 0.0;
  if (parser.is_used("--noise")) {
    noise = parser.get<float>("--noise");
  }

  // holds all the signals to be processed
  std::vector<SLogicAnalogData> signals;

  //
  // Generate a square wave
  //
  if (parser.is_used("--square")) {
    SLogicAnalogData data;
    data.begin_time = 0;
    data.sample_rate = static_cast<uint64_t>(50.0e6);
    gen_square_wave(parser.get<float>("--square"), 50.0, 0.0, 1, -1,
                    data.samples);
    data.num_samples = data.samples.size();
    signals.emplace_back(data);
  }

  //
  // Generate a sin wave
  //
  if (parser.is_used("--sin")) {
    SLogicAnalogData data;
    data.begin_time = 0;
    data.sample_rate = static_cast<uint64_t>(50.0e6);
    gen_sin_wave(parser.get<float>("--sin"), 50.0, 0.0, 1, -1, data.samples);
    data.num_samples = data.samples.size();
    signals.emplace_back(data);
  }

  //
  // Process raw DLA input files
  //
  std::vector<std::string> in_files =
      parser.get<std::vector<std::string>>("--input");

  for (const auto &path : in_files) {
    SLogicAnalogData data = parse_dla_file(path).value_or(SLogicAnalogData());

    SPDLOG_DEBUG("Parsed DLA data. [path={}, n_samples={}]", path,
                 data.num_samples);
    signals.emplace_back(data);
  }

  //
  // Signal processing
  //
  for (const auto &data : signals) {
    std::vector<double> fft;
    process_signal(data.samples, noise, fft);

    SPDLOG_DEBUG(
        "Processed signal. [sample_rate={}, n_samples={}, fft_bins={}]",
        data.sample_rate, data.num_samples, fft.size());

    std::vector<float> bins;
    bins.reserve(fft.size());
    for (size_t i = 0; i < fft.size(); ++i) {
      //
      // resolution = sample_rate / window_size
      //
      bins.push_back(static_cast<float>(i) *
                     static_cast<float>(data.sample_rate) /
                     static_cast<float>(data.num_samples));
      std::cout << bins[i] << ", " << fft[i] << std::endl;
    }
  }
  return 0;
}
