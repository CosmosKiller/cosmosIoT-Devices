#ifndef SECURITY_MODULE_H_
#define SECURITY_MODULE_H_

#include <driver/gpio.h>
#include <esp_err.h>
#include <esp_matter.h>

#define PIR_PIN          GPIO_NUM_1
#define DOORBELL_PIN     GPIO_NUM_0
#define LED_PIN          GPIO_NUM_21
#define DEBOUNCE_TIME_MS 200
#define TRIGGER_TIME_MS  5000

using security_module_cb_t = void (*)(uint16_t endpoint_id, bool occupied, void *user_data);

typedef void *security_module_task_handle_t;

typedef struct {
    struct {
        security_module_cb_t cb = NULL;
        uint16_t endpoint_id;
    } pir_sensor;

    struct {
        security_module_cb_t cb = NULL;
        uint16_t endpoint_id;
    } doorbell;

    void *user_data = NULL;
} security_module_config_t;

typedef enum {
    EVENT_MOTION_START,
    EVENT_MOTION_SUSTAINED,
    EVENT_CALL_REQUEST,
    EVENT_CALL_START,
    EVENT_ALL_END
} security_event_type_t;

typedef struct {
    security_event_type_t type;
    int level;
} security_event_t;

/** Driver Update
 *
 * @brief This API should be called to update the driver for the attribute being updated.
 *        This is usually called from the common `app_attribute_update_cb()`.
 *
 * @param driver_handle Handle to the driver instance. This is usually passed as `priv_data` while creating the endpoint.
 * @param endpoint_id Endpoint ID of the attribute.
 * @param cluster_id Cluster ID of the attribute.
 * @param attribute_id Attribute ID of the attribute.
 * @param val Pointer to `esp_matter_attr_val_t`. Use appropriate elements as per the value type.
 *
 * @return error in case of failure.
 */
esp_err_t security_module_attribute_update(security_module_task_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                           uint32_t attribute_id, esp_matter_attr_val_t *val);

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