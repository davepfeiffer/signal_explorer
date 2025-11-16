#include <X11/Xlib.h>
#include <argparse/argparse.hpp>
#include <iostream>
#include <spdlog/spdlog.h>

#include "gen_square_wave.h"
#include "parse_dla_data.h"
#include "spi_analysis.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include <GLFW/glfw3.h>
#include <iostream>

// Callback to handle GLFW errors
void glfw_error_callback(int error, const char *description) {
  std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

int main(int argc, char *argv[]) {
  spdlog::set_level(spdlog::level::debug);
  argparse::ArgumentParser parser("sig_exp");

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

  float noise = 0.0;
  if (parser.is_used("--noise")) {
    noise = parser.get<float>("--noise");
  }

  // holds all the signals to be processed
  std::vector<SLogicAnalogData> signals;

  std::vector<std::string> signal_names;
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
    signal_names.emplace_back("generated_square_wave");
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
    signal_names.emplace_back("generated_sin_wave");
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
    signal_names.emplace_back(path);
  }

  std::vector<std::vector<float>> signal_times;
  for (const auto &signal : signals) {
    signal_times.emplace_back();
    signal_times.back().reserve(signal.num_samples);
    for (uint64_t i = 0; i < signal.num_samples; ++i) {
      signal_times.back().push_back(i * signal.sample_rate);
    }
    SPDLOG_DEBUG("last_time: {}", signal_times.back().back());
  }

  //
  // Signal processing
  //
  std::vector<std::vector<float>> ffts;
  std::vector<std::vector<float>> bins;
  for (const auto &data : signals) {
    ffts.emplace_back();
    fft_with_noise(data.samples, noise, ffts.back());

    SPDLOG_DEBUG(
        "Processed signal. [sample_rate={}, n_samples={}, fft_bins={}]",
        data.sample_rate, data.num_samples, ffts.back().size());

    bins.emplace_back();
    bins.back().reserve(ffts.back().size());
    for (size_t i = 0; i < ffts.back().size(); ++i) {
      //
      // resolution = sample_rate / window_size
      //
      bins.back().push_back(static_cast<float>(i) *
                            static_cast<float>(data.sample_rate) /
                            static_cast<float>(data.num_samples));
    }
  }

  // Setup error callback
  glfwSetErrorCallback(glfw_error_callback);

  // Initialize GLFW
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  // Setup OpenGL version
#if defined(__APPLE__)
  // GL 3.2 + GLSL 150 (MacOS)
  const char *glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on MacOS
#else
  // GL 3.0 + GLSL 130 (Windows and Linux)
  const char *glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

  // Create window
  GLFWwindow *window =
      glfwCreateWindow(1200, 800, "ImPlot Example", nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(0); // Disable vsync

  // Setup context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImPlot::CreateContext();

  // Setup style
  ImGui::StyleColorsDark();

  // Setup backend
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Main loop
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // Start frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("ImPlot Demo", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoBringToFrontOnFocus);

    const float plot_height = (ImGui::GetIO().DisplaySize.y - 20) / 8.0;

    for (size_t i = 0; i < signal_names.size(); ++i) {
      ImGui::SeparatorText(signal_names[i].c_str());
      const std::string raw_sig_name = signal_names[i] + " -- Raw Signal";
      if (ImPlot::BeginPlot(raw_sig_name.c_str(), ImVec2(-1, plot_height))) {
        ImPlot::SetupAxes("Time (ns)", "Voltage (V)");
        // ImPlot::SetupAxesLimits(0, signal_times[i].back(), -1,
        // 5,ImGuiCond_Always);
        ImPlot::PlotLine("Signal", signal_times[i].data(),
                         signals[i].samples.data(), signals[i].num_samples);
        ImPlot::EndPlot();
      }

      const std::string fft_sig_name = signal_names[i] + " -- FFT";
      if (ImPlot::BeginPlot(fft_sig_name.c_str(), ImVec2(-1, plot_height))) {
        ImPlot::SetupAxes("Frequency (Hz)", "Amplitude");
        ImPlot::PlotLine("FFT", bins[i].data(), ffts[i].data(), ffts[i].size());
        ImPlot::EndPlot();
      }
    }

    ImGui::End();

    // Render
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Swap buffers
    glfwSwapBuffers(window);
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImPlot::DestroyContext();
  ImGui::DestroyContext();
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
