#include "fft_compression.h"

#include <cmath>
#include <queue>
#include <stdexcept>

namespace oel {
namespace {

constexpr double kPi = 3.14159265358979323846;

bool isPowerOfTwo(std::size_t value) {
    return value != 0 && (value & (value - 1)) == 0;
}

}  // namespace

std::vector<ComplexSample> generateSignal(std::size_t sampleCount,
                                          double samplingFrequency,
                                          double blockLength) {
    if (!isPowerOfTwo(sampleCount)) {
        throw std::invalid_argument("sampleCount must be a power of two");
    }
    if (samplingFrequency <= 0.0) {
        throw std::invalid_argument("samplingFrequency must be positive");
    }

    const double samplingPeriod = 1.0 / samplingFrequency;
    std::vector<ComplexSample> samples(sampleCount);

    for (std::size_t i = 0; i < sampleCount; ++i) {
        const double t = static_cast<double>(i + 1) * (blockLength - 1.0) * samplingPeriod;
        const double value = 0.7 * std::sin(2.0 * kPi * 50.0 * t) +
                             std::sin(2.0 * kPi * 120.0 * t);
        samples[i] = {value, 0.0};
    }

    return samples;
}

void fft(std::vector<ComplexSample>& samples, bool inverse) {
    const std::size_t n = samples.size();
    if (!isPowerOfTwo(n)) {
        throw std::invalid_argument("FFT input size must be a power of two");
    }

    std::size_t j = 0;
    for (std::size_t i = 1; i < n; ++i) {
        std::size_t bit = n >> 1;
        while ((j & bit) != 0) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;

        if (i < j) {
            ComplexSample temp = samples[i];
            samples[i] = samples[j];
            samples[j] = temp;
        }
    }

    for (std::size_t length = 2; length <= n; length <<= 1) {
        const double angle = (inverse ? 2.0 : -2.0) * kPi / static_cast<double>(length);
        const double wLengthReal = std::cos(angle);
        const double wLengthImag = std::sin(angle);

        for (std::size_t i = 0; i < n; i += length) {
            double wReal = 1.0;
            double wImag = 0.0;

            for (std::size_t k = 0; k < length / 2; ++k) {
                ComplexSample even = samples[i + k];
                ComplexSample odd = samples[i + k + length / 2];

                const double oddReal = odd.real * wReal - odd.imag * wImag;
                const double oddImag = odd.real * wImag + odd.imag * wReal;

                samples[i + k] = {even.real + oddReal, even.imag + oddImag};
                samples[i + k + length / 2] = {even.real - oddReal, even.imag - oddImag};

                const double nextWReal = wReal * wLengthReal - wImag * wLengthImag;
                wImag = wReal * wLengthImag + wImag * wLengthReal;
                wReal = nextWReal;
            }
        }
    }

    if (inverse) {
        for (ComplexSample& sample : samples) {
            sample.real /= static_cast<double>(n);
            sample.imag /= static_cast<double>(n);
        }
    }
}

double magnitude(const ComplexSample& sample) {
    return std::sqrt(sample.real * sample.real + sample.imag * sample.imag);
}

std::vector<ComplexSample> compressByMagnitude(
    const std::vector<ComplexSample>& frequencySamples,
    double minimumMagnitude) {
    if (minimumMagnitude < 0.0) {
        throw std::invalid_argument("minimumMagnitude cannot be negative");
    }

    std::vector<ComplexSample> compressed = frequencySamples;
    for (ComplexSample& sample : compressed) {
        if (magnitude(sample) < minimumMagnitude) {
            sample = {0.0, 0.0};
        }
    }
    return compressed;
}

std::vector<ComplexSample> transmitThroughQueue(
    const std::vector<ComplexSample>& frequencySamples) {
    std::queue<ComplexSample> queue;
    for (const ComplexSample& sample : frequencySamples) {
        queue.push(sample);
    }

    std::vector<ComplexSample> received;
    received.reserve(frequencySamples.size());
    while (!queue.empty()) {
        received.push_back(queue.front());
        queue.pop();
    }
    return received;
}

std::vector<ComplexSample> reconstructSignal(
    const std::vector<ComplexSample>& transmittedSamples) {
    std::vector<ComplexSample> reconstructed = transmittedSamples;
    fft(reconstructed, true);
    return reconstructed;
}

}  // namespace oel
