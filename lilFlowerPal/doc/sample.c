#include "driver/adc.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

// ----------------- CONFIG -----------------
#define NO_OF_SAMPLES 16 // multisamples per read (>=4 for min/max discard)
#define FILTER_SIZE   10 // moving average window size

// Using IO39 = ADC1_CHANNEL_3 on ESP32
#define WATER_SENSOR_CH ADC_CHANNEL_3
#define ADC_UNIT        ADC_UNIT_1

// ----------------- FILTER STRUCT -----------------
typedef struct {
    int buffer[FILTER_SIZE];
    int index;
    int count;
    long sum;
} moving_avg_t;

void moving_avg_init(moving_avg_t *f)
{
    f->index = 0;
    f->count = 0;
    f->sum = 0;
    for (int i = 0; i < FILTER_SIZE; i++)
        f->buffer[i] = 0;
}

int moving_avg_update(moving_avg_t *f, int new_value)
{
    f->sum -= f->buffer[f->index];
    f->buffer[f->index] = new_value;
    f->sum += new_value;
    f->index = (f->index + 1) % FILTER_SIZE;
    if (f->count < FILTER_SIZE)
        f->count++;
    return (int)(f->sum / f->count);
}

// ----------------- ADC HELPERS -----------------
int read_adc_with_discard(adc_oneshot_unit_handle_t adc_handle, adc_channel_t ch)
{
    int v;
    int min = INT32_MAX, max = INT32_MIN;
    long sum = 0;

    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        adc_oneshot_read(adc_handle, ch, &v);
        if (v < min)
            min = v;
        if (v > max)
            max = v;
        sum += v;
    }

    if (NO_OF_SAMPLES <= 2)
        return (int)(sum / NO_OF_SAMPLES);

    sum -= min;
    sum -= max;
    return (int)(sum / (NO_OF_SAMPLES - 2));
}

int read_sensor_filtered(adc_oneshot_unit_handle_t adc_handle, adc_channel_t ch, moving_avg_t *filter)
{
    int sample = read_adc_with_discard(adc_handle, ch);
    return moving_avg_update(filter, sample);
}

// ----------------- APP MAIN -----------------
void app_main(void)
{
    // 1. Create ADC oneshot handle
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    adc_oneshot_new_unit(&init_config1, &adc1_handle);

    // 2. Configure channel (IO39 = ADC1_CH3)
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_11, // up to ~3.6V
    };
    adc_oneshot_config_channel(adc1_handle, WATER_SENSOR_CH, &config);

    // 3. Init moving average filter
    moving_avg_t water_filter;
    moving_avg_init(&water_filter);

    // 4. Main loop
    while (1) {
        int filtered_value = read_sensor_filtered(adc1_handle, WATER_SENSOR_CH, &water_filter);

        // Optional: map to % (0–100%)
        int percentage = (filtered_value * 100) / 4095;

        printf("ADC Raw Filtered: %d, Water Level: %d%%\n", filtered_value, percentage);

        vTaskDelay(pdMS_TO_TICKS(1000)); // 1 sec update
    }
}
