#!/usr/bin/fish

set -Ux CUDA_PATH /opt/cuda
fish_add_path '/opt/cuda/bin'
fish_add_path '/opt/cuda/nsight_compute'
fish_add_path '/opt/cuda/nsight_systems/bin'
# export PATH


# Set the default host compiler for nvcc. This will need to be switched back
# and forth between the latest and previous GCC version, whatever nvcc
# currently supports.
set -Ux NVCC_CCBIN '/usr/bin/g++-14'
