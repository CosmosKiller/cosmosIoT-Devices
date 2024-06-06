#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "cosmos_socket.h"
#include "feeder_local_control.h"
#include "pet_feeder_tasks_common.h"

#define TAG "Local"

static cosmos_devices_t output_devs[] = {
    {
        .sn = "MOTSv-aaa0000",
        .pin = {23, 0, 0},
        .state = 0,
    },
    {
        .sn = "SKTCc-aaa0000",
        .pin = {22, 0, 0},
        .state = 0,
    },
    {
        .sn = "SKTCc-aaa0001",
        .pin = {21, 0, 0},
        .state = 0,
    },
};
// static const int out_dev_qty = sizeof(output_devs) / sizeof(output_devs[0]);

// Defining button pins
static int button_pins[] = {34, 35, 32};
static const int btn_qty = sizeof(button_pins) / sizeof(button_pins[0]);

static TaskHandle_t button_isr_task;
static BaseType_t is_notified, button_isr_task_woken;
static feeder_local_control_notify_type_e notify_send, notify_recive;

/**
 * @brief Fuction to be excecuted when an ISR is
 * triggred by a button
 *
 * @param arg Arguments which can be passed to
 * the ISR handler
 */
static void feeder_local_control_isr_handler(void *arg)
{
    // Get the device serial number from the argument
    char *dev_serial = (char *)arg;
    button_isr_task_woken = pdFALSE;

    if (strncmp("SKTCc-aaa0000", dev_serial, 13) == 0)
        notify_send = NOTIFY_TYPE_FLOWER_PUMP;
    else if (strncmp("SKTCc-aaa0001", dev_serial, 13) == 0)
        notify_send = NOTIFY_TYPE_WATER_PUMP;
    else if (strncmp("MOTSv-aaa0000", dev_serial, 13) == 0)
        notify_send = NOTIFY_TYPE_PET_FEEDER;

    xTaskNotifyFromISR(button_isr_task, notify_send, eSetValueWithoutOverwrite, &button_isr_task_woken);
    if (button_isr_task_woken)
        portYIELD_FROM_ISR(button_isr_task_woken);
}

/**
 * @brief Reacts to an ISR event and evaluates the
 * recived notify type. Triggers different device
 * based on the notification recived.
 *
 * @param pvParameter Parameter which can be passed to the task.
 */
static void feeder_local_control_task(void *pvParameter)
{
    while (1) {
        notify_recive = NOTIFY_TYPE_PET_DEFAULT;
        is_notified = pdFALSE;

        ESP_LOGI(TAG, "Valor %d\n", notify_recive);

        is_notified = xTaskNotifyWait(0, 0, &notify_recive, portMAX_DELAY);

        if (is_notified) {

            switch (notify_recive) {
            case NOTIFY_TYPE_FLOWER_PUMP:
                ESP_LOGI(TAG, "Valor %d\nk", notify_recive);
                vTaskDelay(4000 / portTICK_PERIOD_MS);
                break;

            case NOTIFY_TYPE_WATER_PUMP:
                ESP_LOGI(TAG, "Valor %d\n", notify_recive);
                vTaskDelay(4000 / portTICK_PERIOD_MS);
                break;

            case NOTIFY_TYPE_PET_FEEDER:
                ESP_LOGI(TAG, "Valor %d\n", notify_recive);
                vTaskDelay(4000 / portTICK_PERIOD_MS);
                break;

            default:
                break;
            }
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void feeder_local_control_task_start(void)
{
    // Configure the ISR in every button
    cosmos_devices_button_monitor(button_pins, btn_qty, feeder_local_control_isr_handler, output_devs);

    // Creates the local feeder control task
    xTaskCreatePinnedToCore(&feeder_local_control_task, "feeder_local_control_task", FEEDER_LOCAL_CONTROL_TASK_STACK_SIZE, NULL, FEEDER_LOCAL_CONTROL_TASK_PRIORITY, &button_isr_task, FEEDER_LOCAL_CONTROL_TASK_CORE_ID);
}