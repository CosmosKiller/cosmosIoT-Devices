#ifndef PUMP_TASK_H_
#define PUMP_TASK_H_

#include <driver/gpio.h>
#include <esp_err.h>
#include <esp_matter.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include "esp_openthread_types.h"
#endif

#define PUMP0 GPIO_NUM_25
#define PUMP1 GPIO_NUM_26
#define PUMP2 GPIO_NUM_27
#define PUMP3 GPIO_NUM_14

#define DEFAULT_POWER false

typedef void *pump_task_handle_t;

typedef struct {
    const int GPIO_PIN_VALUE; /*<! GPIO pin associated with the pump */
} gpio_pump_t;

/**
 * @brief Configuration structure for the pump
 *
 */
typedef struct {
    uint16_t endpoint_id; /*!< Endpoint ID associated with the pump */
    int gpio;             /*!< GPIO pin associated with the pump */
} pump_task_config_t;

/**
 *
 * @brief This initializes the pump driver
 *
 * @param pPump Pointer to the GPIO configuration structure
 *
 * @return ESP_OK on success
 */
esp_err_t pump_task_init(const gpio_pump_t *pPump);

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
esp_err_t pump_task_attribute_update(pump_task_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                     uint32_t attribute_id, esp_matter_attr_val_t *val);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#define ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG() \
    {                                         \
        .radio_mode = RADIO_MODE_NATIVE,      \
    }

#define ESP_OPENTHREAD_DEFAULT_HOST_CONFIG()               \
    {                                                      \
        .host_connection_mode = HOST_CONNECTION_MODE_NONE, \
    }

#define ESP_OPENTHREAD_DEFAULT_PORT_CONFIG() \
    {                                        \
        .storage_partition_name = "nvs",     \
        .netif_queue_size = 10,              \
        .task_queue_size = 10,               \
    }
#endif

#endif /* PUMP_TASK_H_ */