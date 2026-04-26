# FFT Based Compression in FreeRTOS

This repository is based on the Open Ended Lab report titled **"FFT Based Compression in FreeRTOS"** for a Real Time Embedded Systems course. The project demonstrates how a time-domain signal can be transformed into the frequency domain with a Fast Fourier Transform (FFT), transmitted as frequency-domain samples through a queue, and reconstructed with an inverse FFT (IFFT).

The original report targets Arduino with FreeRTOS. This repository keeps that embedded idea while also providing a portable C++ implementation that can be compiled and tested on a desktop machine.

## Repository Layout

```text
source/
  FFTBasedCompressionFreeRTOS.ino  Arduino/FreeRTOS sketch adapted from the report appendix
  fft_compression.cpp              Portable FFT, compression, queue simulation, and reconstruction logic
  fft_compression.h                Public interface for the portable source

test/
  test_fft_compression.cpp         Dependency-free C++ tests for the portable implementation
```

## Project Summary

The lab focuses on FFT-based signal compression in a real-time embedded system. A sender task generates a sinusoidal signal, transforms it into frequency-domain values, and sends those values through a FreeRTOS queue. A receiver task collects the transmitted values and applies the inverse FFT to recover the original signal.

The core flow is:

1. Generate a sampled signal using sine waves at 50 Hz and 120 Hz.
2. Represent each sample as a complex number with a real component and zero imaginary component.
3. Apply FFT to convert the signal from the time domain to the frequency domain.
4. Compress the frequency-domain data by discarding low-magnitude components.
5. Send the remaining frequency-domain samples through a queue.
6. Apply IFFT at the receiver side to reconstruct the time-domain signal.
7. Compare the reconstructed signal with the original signal.

## Why FFT Helps Compression

Many signals contain most of their useful information in a small number of frequency components. FFT exposes those components. Once the signal is in the frequency domain, smaller or less important frequency components can be removed or quantized, reducing the amount of data that must be stored or transmitted.

In this project, `compressByMagnitude` performs a simple version of that idea: any frequency sample below a selected magnitude threshold is replaced with zero.

## Source Code Notes

`source/FFTBasedCompressionFreeRTOS.ino` is the Arduino/FreeRTOS version. It creates a sender task, a receiver task, and a queue. The sender generates the signal, computes the FFT, and writes values to the queue. The receiver reads values from the queue and applies the inverse transform.

`source/fft_compression.cpp` is a portable C++ version designed for testing. It contains:

- `generateSignal`: creates the sampled 50 Hz and 120 Hz sinusoidal input.
- `fft`: performs an in-place radix-2 FFT or IFFT.
- `compressByMagnitude`: drops small frequency components.
- `transmitThroughQueue`: simulates FIFO queue transmission.
- `reconstructSignal`: applies the inverse FFT to recover time-domain samples.

The portable version intentionally avoids Arduino and FreeRTOS headers so tests can run locally.

## Running the Tests

From the repository root:

```powershell
g++ -std=c++17 source/fft_compression.cpp test/test_fft_compression.cpp -o test/fft_compression_tests.exe
.\test\fft_compression_tests.exe
```

Expected output:

```text
All FFT compression tests passed.
```

## Test Coverage

The tests verify that:

- FFT followed by IFFT reconstructs the original samples.
- Queue transmission preserves FIFO ordering.
- Compression removes only components below the threshold.
- The report-style signal pipeline reconstructs correctly when no frequencies are discarded.
- Invalid FFT input sizes are rejected.

## Embedded Target

To run the Arduino sketch on hardware, install a FreeRTOS-compatible Arduino environment and open `source/FFTBasedCompressionFreeRTOS.ino` in the Arduino IDE. The sketch expects the FreeRTOS headers and APIs used in the report:

- `FreeRTOS.h`
- `task.h`
- `queue.h`

The serial monitor prints the generated samples, transmitted FFT values, and reconstructed IFFT output.

## Limitations

The compression method is intentionally simple. Real compression systems usually include threshold tuning, quantization, framing, metadata, and error handling. The report uses a 4-sample FFT for demonstration, which is useful for understanding the algorithm but too small for practical signal compression.

## Report Context

The report describes this as an open-ended lab for real-time embedded systems. Its main learning goals are:

- Implement FFT and IFFT in C/C++.
- Generate a sampled sinusoidal signal.
- Transmit data between real-time tasks using queues.
- Verify that the received and reconstructed signal matches the original signal.
