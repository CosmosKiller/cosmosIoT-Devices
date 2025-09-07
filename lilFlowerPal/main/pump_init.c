/**
 * @file pump_init.c
 * @author Marcel Nahir Samur (mnsamur2014@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-06-18
 *
 * @copyright Copyright (c) 2024
 *
 */

// Include standard libraries
#include <stdio.h>
#include <string.h>

// Include ESP-IDF libraries
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Include ESP-MATTER libraries
#include <device.h>
#include <esp_matter.h>

// Include project libraries
#include "cosmos_devices.h"
#include "cosmos_power.h"
#include "main_tasks_common.h"
#include "pump_init.h"

static const char *TAG = "pump_task";
extern uint16_t pump0_endpoint_id;
extern uint16_t pump1_endpoint_id;
extern uint16_t pump2_endpoint_id;
extern uint16_t pump3_endpoint_id;

pump_task_config_t pump_data[] = {
    {
        .endpoint_id = pump0_endpoint_id,
        .gpio = PUMP0,
    },
    {
        .endpoint_id = pump1_endpoint_id,
        .gpio = PUMP1,
    },
    {
        .endpoint_id = pump2_endpoint_id,
        .gpio = PUMP2,
    },
    {
        .endpoint_id = pump3_endpoint_id,
        .gpio = PUMP3,
    },
};

/**
 * @brief Changes the state of the pump based on the attribute value.
 *
 * @param val value of the attribute to be changed
 * @param pump_gpio GPIO pin associated with the pump
 * @return esp_err_t
 */
static esp_err_t pump_task_pump_set_on_off(esp_matter_attr_val_t *val, const int *pump_gpio)
{
    /* print val as text */
    ESP_LOGI(TAG, "Changing the pump state to %s!", val->val.b ? "ON" : "OFF");
    gpio_set_level(&pump_gpio, val->val.b);

    return ESP_OK;
}

esp_err_t pump_task_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                     uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    esp_err_t err = ESP_OK;

    /* Do stuff here */
    for (size_t i = 0; i < QTY(pump_data); i++) {
        if (endpoint_id == pump_data[i].endpoint_id) {
            if (cluster_id == OnOff::Id) {
                if (attribute_id == OnOff::Attributes::OnOff::Id) {
                    err = pump_task_pump_set_on_off(val, pump_data[i]->gpio);
                }
            }
        }
    }
    return err;
}

/**
 * @brief To-Do
 *
 */
pump_task_handle_t pump_task_init()
{
    for (size_t i = 0; i < QTY(pump_data); i++) {
        ESP_LOGI(TAG, "Initializing pump at GPIO %d", pump_data[i].gpio);

        esp_rom_gpio_pad_select_gpio(pump_data[i].gpio);
        gpio_set_direction(pump_data[i].gpio, GPIO_MODE_OUTPUT);
        gpio_intr_disable(pump_data[i].gpio);
        gpio_set_level(pump_data[i].gpio, 0); // Ensure pump is off at start
    }

    return ESP_OK;
}

esp_err_t pump_task_pump_set_defaults(pump_task_config_t *pPump)
{
    esp_err_t err = ESP_OK;
    node_t *node = node::get();
    endpoint_t *endpoint = endpoint::get(node, endpoint_id);
    cluster_t *cluster = NULL;
    attribute_t *attribute = NULL;
    esp_matter_attr_val_t val = esp_matter_invalid(NULL);

    /* Setting power */
    cluster = cluster::get(endpoint, OnOff::Id);
    attribute = attribute::get(cluster, OnOff::Attributes::OnOff::Id);
    attribute::get_val(attribute, &val);
    err |= app_driver_light_set_on_off(&val);

    return err;
}