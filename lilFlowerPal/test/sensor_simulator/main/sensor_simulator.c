#include "driver/dac.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>
#include <stdio.h>

#define DAC1 DAC_CHANNEL_1 // GPIO25
#define DAC2 DAC_CHANNEL_2 // GPIO26

#define DAC_MAX_MV 3300

// Soil moisture range: ~1000 mV (wet) .. 3000 mV (dry)
#define SOIL_MIN_MV 1000
#define SOIL_MAX_MV 3000

// Water level range: ~500 mV (empty) .. 2500 mV (full)
#define LEVEL_MIN_MV 500
#define LEVEL_MAX_MV 2500

// Curve resolution
#define STEPS 200

static void set_dac_mv(dac_channel_t ch, int mv)
{
    if (mv < 0)
        mv = 0;
    if (mv > DAC_MAX_MV)
        mv = DAC_MAX_MV;

    int dac_val = (mv * 255 + DAC_MAX_MV / 2) / DAC_MAX_MV; // scale to 0–255
    dac_output_enable(ch);
    dac_output_voltage(ch, dac_val);
}

void app_main(void)
{
    printf("Starting multi-channel sensor simulator...\n");

    int step = 0;
    while (1) {
        // ---- Soil moisture: sinusoidal 1–3 V ----
        float x = (2.0f * M_PI * step) / STEPS;
        float sine_val = (sinf(x) + 1.0f) / 2.0f; // 0..1
        int soil_mv = (int)(SOIL_MIN_MV + sine_val * (SOIL_MAX_MV - SOIL_MIN_MV));
        set_dac_mv(DAC1, soil_mv);

        // ---- Water level: triangular 0.5–2.5 V ----
        float tri_phase = (float)step / STEPS;                                         // 0..1
        float tri_val = tri_phase < 0.5f ? tri_phase * 2.0f : 2.0f - tri_phase * 2.0f; // 0..1
        int level_mv = (int)(LEVEL_MIN_MV + tri_val * (LEVEL_MAX_MV - LEVEL_MIN_MV));
        set_dac_mv(DAC2, level_mv);

        // Debug print
        printf("Soil moisture ~ %d mV | Water level ~ %d mV\n", soil_mv, level_mv);

        // advance
        step = (step + 1) % STEPS;

        vTaskDelay(pdMS_TO_TICKS(50)); // ~10 s per cycle
    }
}
