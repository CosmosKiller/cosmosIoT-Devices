/**
 * @file soil_moisture_task.c
 * @author Marcel Nahir Samur (mnsamur2014@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-06-13
 *
 * @copyright Copyright (c) 2024
 *
 */
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "cosmos_devices.h"
#include "cosmos_sensor.h"
#include "main_tasks_common.h"

cosmos_sensor_t soil_moisture_sensors[] = {
    {
        .pin_num = GPIO_NUM_34,
        .snr_chn = ADC_CHANNEL_6,
        .type = SNR_TYPE_SM,
        .snr_handle = NULL,
    },
    {
        .pin_num = GPIO_NUM_35,
        .snr_chn = ADC_CHANNEL_7,
        .type = SNR_TYPE_SM,
        .snr_handle = NULL,
    },
    {
        .pin_num = GPIO_NUM_32,
        .snr_chn = ADC_CHANNEL_4,
        .type = SNR_TYPE_SM,
        .snr_handle = NULL,
    },
    {
        .pin_num = GPIO_NUM_33,
        .snr_chn = ADC_CHANNEL_5,
        .type = SNR_TYPE_SM,
        .snr_handle = NULL,
    },
};

/**
 * @brief Trigger moisture sensor readings
 *
 * @param pvParameters Parameter which can be passed to the task.
 */
static void soil_moisture_task(void *pvParameters)
{
    int snr_qty = QTY(soil_moisture_sensors);
    uint32_t current_reading;
    while (1) {
        cosmos_sensor_adc_read_raw(soil_moisture_sensors, snr_qty);

        for (int idx = 0; idx < snr_qty; idx++) {
            current_reading = COSMOS_MAP(soil_moisture_sensors[idx].reading, 3000, 1500, 0, 100);

            /*
            As the soil gets wetter, the output value decreases, and as it gets drier,
            the output value increases. When powered at 5V, the output ranges from
            about 1.5V (for wet soil) to 3V (for dry soil).

            Source: https://lastminuteengineers.com/capacitive-soil-moisture-sensor-arduino/
            */

            printf("Moisture sensor %d: %ld\n", idx + 1, current_reading);
            vTaskDelay(pdMS_TO_TICKS(250));
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void soil_moisture_task_start(void)
{
    xTaskCreatePinnedToCore(&soil_moisture_task, "soil_moisture_task", SOIL_MOISTURE_TASK_STACK_SIZE, NULL, SOIL_MOISTURE_TASK_PRIORITY, NULL, SOIL_MOISTURE_TASK_CORE_ID);
}