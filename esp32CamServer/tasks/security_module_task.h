#ifndef SECURITY_MODULE_H_
#define SECURITY_MODULE_H_

#include <driver/gpio.h>
#include <esp_err.h>
#include <esp_matter.h>

#define PIR_PIN         GPIO_NUM_1
#define CONTACT_PIN     GPIO_NUM_2
#define PANIC_PIN       GPIO_NUM_3
#define TRIGGER_TIME_MS 5000

using security_module_cb_t = void (*)(uint16_t endpoint_id, bool occupied, void *user_data);

typedef struct {
    struct {
        security_module_cb_t cb = NULL;
        uint16_t endpoint_id;
    } pir_sensor;

    struct {
        security_module_cb_t cb = NULL;
        uint16_t endpoint_id;
    } contact_sensor;

    void *user_data = NULL;
} security_module_config_t;

/**
 * @brief Initialize sensor driver. This function should be called only once
 *
 * @param pConfig sensor configurations. This should last for the lifetime of the driver
 *               as driver layer do not make a copy of this object.
 *
 * @return esp_err_t - ESP_OK on success,
 *                     ESP_ERR_INVALID_ARG if pConfig is NULL
 *                     ESP_ERR_INVALID_STATE if driver is already initialized
 *                     appropriate error code otherwise
 */
esp_err_t security_module_task_init(security_module_config_t *pConfig);

#endif // SECURITY_MODULE_H_