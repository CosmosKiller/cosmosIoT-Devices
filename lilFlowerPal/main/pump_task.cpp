/**
 * @file pump_task.c
 * @author Marcel Nahir Samur (mnsamur2014@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-06-18
 *
 * @copyright Copyright (c) 2024
 *
 */

// Include ESP-IDF libraries
#include <esp_log.h>

// Include ESP-MATTER libraries
#include <esp_matter.h>

// Include project libraries
#include <pump_task.h>

using namespace chip::app::Clusters;
using namespace esp_matter;

static const char *TAG = "pump_task";

extern pump_task_config_t pumps_config[PUMP_QTY];

/**
 * @brief Changes the state of the pump based on the attribute value.
 *
 * @param val value of the attribute to be changed
 * @param pump_gpio GPIO pin associated with the pump
 * @return esp_err_t
 */
static esp_err_t pump_task_pump_set_on_off(esp_matter_attr_val_t *val, const int pump_gpio)
{
    /* print val as text */
    ESP_LOGI(TAG, "Changing the pump state to %s!", val->val.b ? "ON" : "OFF");
    gpio_set_level((gpio_num_t)pump_gpio, val->val.b);

    return ESP_OK;
}

esp_err_t pump_task_attribute_update(pump_task_handle_t pump_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                     uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    esp_err_t err = ESP_OK;

    /* Look for the correct endpoint_id */
    for (int i = 0; i < PUMP_QTY; i++) {
        if (endpoint_id == pumps_config[i].endpoint_id) {
            if (cluster_id == OnOff::Id) {
                if (attribute_id == OnOff::Attributes::OnOff::Id) {
                    err = pump_task_pump_set_on_off(val, pumps_config[i].gpio);
                }
            }
        }
    }
    return err;
}

esp_err_t pump_task_init(const gpio_pump_t *pPump)
{
    for (size_t i = 0; i < PUMP_QTY; i++) {
        ESP_LOGI(TAG, "Initializing pump at GPIO %d", pPump[i].GPIO_PIN_VALUE);

        esp_rom_gpio_pad_select_gpio((gpio_num_t)pPump[i].GPIO_PIN_VALUE);
        gpio_set_direction((gpio_num_t)pPump[i].GPIO_PIN_VALUE, GPIO_MODE_OUTPUT);
        gpio_intr_disable((gpio_num_t)pPump[i].GPIO_PIN_VALUE);
        gpio_set_level((gpio_num_t)pPump[i].GPIO_PIN_VALUE, 0); // Ensure pump is off at start
    }

    return ESP_OK;
}