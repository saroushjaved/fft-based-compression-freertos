#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include <math.h>
#include <stdlib.h>

#define TRSIZ 4

TaskHandle_t senderTask;
TaskHandle_t receiverTask;
QueueHandle_t fftQueue;

static void ditFft(double* data, int isign) {
    double wr, wi, wpr, wpi, wtemp, theta;
    double tempr, tempi;
    int n = TRSIZ * 2;
    int j = 1;

    for (int i = 1; i < n; i += 2) {
        if (j > i) {
            tempr = data[j - 1];
            data[j - 1] = data[i - 1];
            data[i - 1] = tempr;

            tempr = data[j];
            data[j] = data[i];
            data[i] = tempr;
        }

        int m = n >> 1;
        while (m >= 2 && j > m) {
            j -= m;
            m >>= 1;
        }
        j += m;
    }

    int mmax = 2;
    while (n > mmax) {
        int istep = mmax << 1;
        theta = isign * (6.28318530717959 / mmax);
        wtemp = sin(0.5 * theta);
        wpr = -2.0 * wtemp * wtemp;
        wpi = sin(theta);
        wr = 1.0;
        wi = 0.0;

        for (int m = 1; m < mmax; m += 2) {
            for (int i = m; i <= n; i += istep) {
                j = i + mmax;
                tempr = wr * data[j - 1] - wi * data[j];
                tempi = wr * data[j] + wi * data[j - 1];
                data[j - 1] = data[i - 1] - tempr;
                data[j] = data[i] - tempi;
                data[i - 1] += tempr;
                data[i] += tempi;
            }

            wtemp = wr;
            wr += wtemp * wpr - wi * wpi;
            wi += wtemp * wpi + wi * wpr;
        }

        mmax = istep;
    }
}

static void sender(void*) {
    for (;;) {
        const float fs = 1000.0f;
        const float period = 1.0f / fs;
        const float blockLength = 1500.0f;
        double data[2 * TRSIZ];

        for (int i = 0; i < TRSIZ; ++i) {
            float t = (i + 1) * (blockLength - 1.0f) * period;
            double signal = 0.7 * sin(2.0 * 3.141592653589793 * 50.0 * t) +
                            sin(2.0 * 3.141592653589793 * 120.0 * t);
            double noise = (double)(rand() % 100) / 1000.0;
            data[2 * i] = signal + noise;
            data[(2 * i) + 1] = 0.0;
        }

        Serial.println("Generated time-domain signal:");
        for (int i = 0; i < 2 * TRSIZ; ++i) {
            Serial.print(data[i], 6);
            Serial.print(" ");
        }
        Serial.println();

        ditFft(data, -1);

        Serial.println("FFT values sent through queue:");
        for (int i = 0; i < 2 * TRSIZ; ++i) {
            xQueueSendToBack(fftQueue, &data[i], pdMS_TO_TICKS(30));
            Serial.print(data[i], 6);
            Serial.print(" ");
            vTaskDelay(pdMS_TO_TICKS(30));
        }
        Serial.println();

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void receiver(void*) {
    for (;;) {
        double received[2 * TRSIZ];

        for (int i = 0; i < 2 * TRSIZ; ++i) {
            xQueueReceive(fftQueue, &received[i], portMAX_DELAY);
        }

        ditFft(received, 1);

        Serial.println("Reconstructed time-domain signal:");
        for (int i = 0; i < 2 * TRSIZ; ++i) {
            Serial.print(received[i] / TRSIZ, 6);
            Serial.print(" ");
        }
        Serial.println();
    }
}

void setup() {
    Serial.begin(9600);
    while (!Serial) {
    }

    fftQueue = xQueueCreate(10, sizeof(double));
    xTaskCreate(sender, "Transmitter", 3000, NULL, 1, &senderTask);
    xTaskCreate(receiver, "Receiver", 3000, NULL, 2, &receiverTask);
    vTaskStartScheduler();
}

void loop() {
}
