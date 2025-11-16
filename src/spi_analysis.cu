#include <cuda_runtime.h>
#include <cufft.h>
#include <curand_kernel.h>
#include <ostream>
#include <stdio.h>
#include <time.h>

#include <iostream>
#include <vector>

// google search AI slop for cuda error reporting
#define CUDA_CHECK(call)                                                       \
  do {                                                                         \
    cudaError_t err = call;                                                    \
    if (err != cudaSuccess) {                                                  \
      fprintf(stderr, "CUDA Error: %s in %s at line %d\n",                     \
              cudaGetErrorString(err), __FILE__, __LINE__);                    \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  } while (0)

__global__ void setup_kernel(curandState *state, unsigned long seed) {
  int idx = blockIdx.x * blockDim.x + threadIdx.x;
  curand_init(seed, idx, 0, &state[idx]);
}

// kernel to add noise to a signal
__global__ void add_noise(float *data, curandState *state, int n,
                          float noise_scale //
) {
  int idx = blockIdx.x * blockDim.x + threadIdx.x;

  if (idx < n) {
    data[idx] += curand_normal(&state[idx]) * noise_scale;
  }
}

// kernel to compute the complex magnitude for a FFT bin
__global__ void complex_mag(cufftComplex *data, float *output, int n) {
  int idx = blockIdx.x * blockDim.x + threadIdx.x;
  if (idx < n) {
    output[idx] = sqrtf(data[idx].x * data[idx].x + data[idx].y * data[idx].y);
  }
}

// GPU function to take the derivative of a signal
__device__ void derivative(float *signal, int samples, float *output //
) {
  int idx = blockIdx.x * blockDim.x + threadIdx.x;
  if (idx < samples - 1) {
    output[idx] = signal[idx + 1] - signal[idx];
  }
}

// incomplete function to digitize an analog singal
__global__ void digitize(float *signal, int samples, float logic_high,
                         float thres_low, float thres_hi, float *output //
) {
  int idx = blockIdx.x * blockDim.x + threadIdx.x;
  if (idx < samples) {
  }
}

//
// Top level CUDA function that will add guassian noise to a signal and perform
// an FFT on the result
//
void fft_with_noise(const std::vector<float> &data, float noise_scale,
                    std::vector<float> &result //
) {
  result.resize(data.size() / 2 + 1);
  std::vector<cufftComplex> fft_result(data.size() / 2 + 1);

  const int thrds_per_blk = 256;
  const int blks_per_grid = (data.size() + thrds_per_blk - 1) / thrds_per_blk;
  const int fft_blks_per_grid =
      (result.size() + thrds_per_blk - 1) / thrds_per_blk;

  // Device arrays
  float *gpu_array;
  curandState *gpu_rand_state;
  cufftComplex *gpu_fft;
  float *gpu_fft_mag;

  // Allocate GPU memory
  CUDA_CHECK(cudaMalloc(&gpu_array, data.size() * sizeof(float)));
  CUDA_CHECK(cudaMalloc(&gpu_rand_state, data.size() * sizeof(curandState)));
  CUDA_CHECK(cudaMalloc(&gpu_fft, result.size() * sizeof(cufftComplex)));
  CUDA_CHECK(cudaMalloc(&gpu_fft_mag, result.size() * sizeof(float)));

  // Copy data to device
  CUDA_CHECK(cudaMemcpy(gpu_array, data.data(), data.size() * sizeof(float),
                        cudaMemcpyHostToDevice));

  // Initialize random states
  setup_kernel<<<blks_per_grid, thrds_per_blk>>>(gpu_rand_state, time(NULL));
  cudaDeviceSynchronize();

  add_noise<<<blks_per_grid, thrds_per_blk>>>(gpu_array, gpu_rand_state,
                                              data.size(), noise_scale);
  cudaDeviceSynchronize();

  // copy data back to check
  CUDA_CHECK(cudaMemcpy((void *)data.data(), gpu_array,
                        data.size() * sizeof(float), cudaMemcpyDeviceToHost));
  cudaDeviceSynchronize();

  // Create FFT handle
  cufftHandle plan;
  cufftPlan1d(&plan, data.size(), CUFFT_R2C, 1);

  // Execute FFT
  cufftExecR2C(plan, gpu_array, gpu_fft);
  cudaDeviceSynchronize();

  complex_mag<<<fft_blks_per_grid, thrds_per_blk>>>(gpu_fft, gpu_fft_mag,
                                                    result.size());

  cudaDeviceSynchronize();

  // Copy FFT results back to host
  CUDA_CHECK(cudaMemcpy(fft_result.data(), gpu_fft,
                        fft_result.size() * sizeof(cufftComplex),
                        cudaMemcpyDeviceToHost));
  CUDA_CHECK(cudaMemcpy(result.data(), gpu_fft_mag,
                        result.size() * sizeof(float), cudaMemcpyDeviceToHost));

  cudaDeviceSynchronize();
  for (size_t i = 0; i < result.size(); ++i) {
    result[i] = std::sqrtf(fft_result[i].x * fft_result[i].x +
                           fft_result[i].y * fft_result[i].y);
  }

  // Destroy cuFFT plan
  cufftDestroy(plan);

  // Cleanup
  cudaFree(gpu_array);
  cudaFree(gpu_rand_state);
  cudaFree(gpu_fft);
  cudaFree(gpu_fft_mag);
}
