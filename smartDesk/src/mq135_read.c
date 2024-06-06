#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "cosmos_devices.h"
#include "cosmos_sensor.h"
#include "mq135_read.h"
#include "smart_desk_tasks_common.h"

static float voltage, mq135_reading;

// Sensors
static cosmos_devices_t sensors = {
    .sn = "SNRTh-aaa0000",
    .pin = {36, 0, 0},
    .state = 0,
};

static cosmos_sensor_t mq135 = {
    .pin_num = sensors.pin,
    .type = SNR_TYPE_PO,
};

/**
 * @brief Reads voltage values from MQ135
 * and converts it into CO2 ppm values.
 *
 * @param pvParameter Parameter which can be passed to the ISR handler.
 */
static void mq135_read_task(void *pvParameter)
{
    while (1) {
        voltage = cosmos_sensor_adc_read(&mq135, 1);
        mq135_reading = (pow(10, ((log10(voltage / 1100) - 2.013) / -0.66)));

        printf("Co2: %.2fppm\tVoltage: %.2fmV\n", mq135_reading, voltage);

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void mq135_read_task_start(void)
{
    xTaskCreatePinnedToCore(&mq135_read_task, "mq135_read_task", MQ135_READ_TASK_STACK_SIZE, NULL, MQ135_READ_TASK_PRIORITY, NULL, MQ135_READ_TASK_CORE_ID);
}