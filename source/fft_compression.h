#ifndef FFT_COMPRESSION_H
#define FFT_COMPRESSION_H

#include <cstddef>
#include <vector>

namespace oel {

struct ComplexSample {
    double real;
    double imag;
};

std::vector<ComplexSample> generateSignal(std::size_t sampleCount,
                                          double samplingFrequency,
                                          double blockLength);

void fft(std::vector<ComplexSample>& samples, bool inverse);

std::vector<ComplexSample> compressByMagnitude(
    const std::vector<ComplexSample>& frequencySamples,
    double minimumMagnitude);

std::vector<ComplexSample> transmitThroughQueue(
    const std::vector<ComplexSample>& frequencySamples);

std::vector<ComplexSample> reconstructSignal(
    const std::vector<ComplexSample>& transmittedSamples);

double magnitude(const ComplexSample& sample);

}  // namespace oel

#endif
