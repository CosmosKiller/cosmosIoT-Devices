#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "cosmos_light.h"
#include "pir_sensor.h"
#include "smart_desk_tasks_common.h"

#define PIR_PIN 26

// LED devices info
static cosmos_devices_t lsc_dev = {
    .sn = "LSCSc-aaa0000",
    .pin = {27, 14, 12},
    .state = 0,
};

// LED configuration info
static cosmos_light_info_t lsc_info = {
    .pDevice = &lsc_dev,
    .rgb_values = "050/255/255/255/",
    .channel = {LEDC_CHANNEL_0, LEDC_CHANNEL_1, LEDC_CHANNEL_2},
    .mode = LEDC_HIGH_SPEED_MODE,
    .timer_index = LEDC_TIMER_0,
};

/**
 * @brief PIR Sensor configuration.
 *
 * Just for testing.
 *
 * @param pvParameter parameter which can be passed to the task.
 */
static void pir_sensor_task(void *pvParameter)
{
    gpio_pad_select_gpio(PIR_PIN);
    gpio_set_direction(PIR_PIN, GPIO_MODE_INPUT);

    while (1) {
        int pir_value = gpio_get_level(PIR_PIN);

        if (pir_value) {
            printf("Movement detected! LED ON.\n");
            cosmos_light_control("LSCSc-aaa0000", "100/255/255/255/", &lsc_info, 1);
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        } else {
            cosmos_light_control("LSCSc-aaa0000", "000/000/000/000/", &lsc_info, 1);
        }
    }
}

void pir_sensor_task_start(void)
{
    xTaskCreatePinnedToCore(&pir_sensor_task, "pir_sensor_task", PIR_SENSOR_TASK_STACK_SIZE, NULL, PIR_SENSOR_TASK_PRIORITY, NULL, PIR_SENSOR_TASK_CORE_ID);
}
