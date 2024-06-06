#include "dht.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "cosmos_devices.h"
#include "dht22_read.h"
#include "smart_desk_tasks_common.h"

// Float varibles to store humidity an temperature
static float humidity, temperature;

// Sensors
static cosmos_devices_t sensors = {
    .sn = "SNRTh-aaa00001",
    .pin = {18, 0, 0},
    .state = 0,
};

/**
 * @brief Reads float values using the DHT22.
 * Prints values in stdout.
 *
 * @param pvParameter Parameter which can be passed to the task.
 */
static void dht22_read_task(void *pvParameter)
{
    while (1) {
        dht_read_float_data(DHT_TYPE_AM2301, sensors.pin[0], &humidity, &temperature);

        printf("Temperature: %.1f\tHumidity: %.1f\n", temperature, humidity);

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void dht22_read_task_start(void)
{
    xTaskCreatePinnedToCore(&dht22_read_task, "dht22_read_task", DHT22_READ_TASK_STACK_SIZE, NULL, DHT22_READ_TASK_PRIORITY, NULL, DHT22_READ_TASK_CORE_ID);
}
