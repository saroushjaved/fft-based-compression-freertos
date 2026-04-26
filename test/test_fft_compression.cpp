#include "../source/fft_compression.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {

constexpr double kTolerance = 1e-9;

void assertNear(double actual, double expected) {
    assert(std::fabs(actual - expected) < kTolerance);
}

void testFftThenIfftRecoversOriginalSamples() {
    std::vector<oel::ComplexSample> samples = {
        {0.0, 0.0},
        {1.0, 0.0},
        {4.0, 0.0},
        {9.0, 0.0},
    };
    std::vector<oel::ComplexSample> original = samples;

    oel::fft(samples, false);
    oel::fft(samples, true);

    for (std::size_t i = 0; i < samples.size(); ++i) {
        assertNear(samples[i].real, original[i].real);
        assertNear(samples[i].imag, original[i].imag);
    }
}

void testQueueTransmissionPreservesFifoOrder() {
    std::vector<oel::ComplexSample> frequencySamples = {
        {14.0, 0.0},
        {-4.0, 8.0},
        {-6.0, 0.0},
        {-4.0, -8.0},
    };

    std::vector<oel::ComplexSample> received =
        oel::transmitThroughQueue(frequencySamples);

    assert(received.size() == frequencySamples.size());
    for (std::size_t i = 0; i < received.size(); ++i) {
        assertNear(received[i].real, frequencySamples[i].real);
        assertNear(received[i].imag, frequencySamples[i].imag);
    }
}

void testCompressionRemovesSmallFrequencyComponents() {
    std::vector<oel::ComplexSample> frequencySamples = {
        {10.0, 0.0},
        {0.01, 0.01},
        {-3.0, 4.0},
        {0.001, 0.0},
    };

    std::vector<oel::ComplexSample> compressed =
        oel::compressByMagnitude(frequencySamples, 0.1);

    assertNear(compressed[0].real, 10.0);
    assertNear(compressed[0].imag, 0.0);
    assertNear(compressed[1].real, 0.0);
    assertNear(compressed[1].imag, 0.0);
    assertNear(compressed[2].real, -3.0);
    assertNear(compressed[2].imag, 4.0);
    assertNear(compressed[3].real, 0.0);
    assertNear(compressed[3].imag, 0.0);
}

void testReportPipelineReconstructsWhenNoFrequenciesAreDiscarded() {
    std::vector<oel::ComplexSample> samples = oel::generateSignal(4, 1000.0, 1500.0);
    std::vector<oel::ComplexSample> expected = samples;

    oel::fft(samples, false);
    std::vector<oel::ComplexSample> compressed =
        oel::compressByMagnitude(samples, 0.0);
    std::vector<oel::ComplexSample> transmitted =
        oel::transmitThroughQueue(compressed);
    std::vector<oel::ComplexSample> reconstructed =
        oel::reconstructSignal(transmitted);

    for (std::size_t i = 0; i < reconstructed.size(); ++i) {
        assertNear(reconstructed[i].real, expected[i].real);
        assertNear(reconstructed[i].imag, 0.0);
    }
}

void testInvalidSizesAreRejected() {
    std::vector<oel::ComplexSample> samples = {
        {1.0, 0.0},
        {2.0, 0.0},
        {3.0, 0.0},
    };

    bool rejected = false;
    try {
        oel::fft(samples, false);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }

    assert(rejected);
}

}  // namespace

int main() {
    testFftThenIfftRecoversOriginalSamples();
    testQueueTransmissionPreservesFifoOrder();
    testCompressionRemovesSmallFrequencyComponents();
    testReportPipelineReconstructsWhenNoFrequenciesAreDiscarded();
    testInvalidSizesAreRejected();

    std::cout << "All FFT compression tests passed.\n";
    return 0;
}
