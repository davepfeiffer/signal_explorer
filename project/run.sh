#!/usr/bin/sh

make
./assignment.exe 256 --input ../dla_data/3_375mhz_data/analog_0.bin > dla_clk__fft_3_375mhz.csv
./assignment.exe 256 --sin 6 > sin_fft_6mhz.csv
./assignment.exe 256 --square 6 -n 1 > square_fft_6mhz_1_0_noise.csv
./assignment.exe 256 --square 6 -n 0.1 > square_fft_6mhz_0_1_noise.csv
./assignment.exe 256 --square 6 -n 0.1 > square_fft_6mhz.csv
./assignment.exe 256 --square 6 > square_fft_6mhz.csv

