/**
 * @file main.c
 * @author Marcel Nahir Samur (mnsamur2014@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-06-09
 *
 * @copyright Copyright (c) 2024
 *
 */

// Include ESP-IDF libraries
#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Include ESP-MATTER libraries
#include <esp_matter.h>
#include <esp_matter_ota.h>

// Include project libraries
#include <matter_task.h>
#include <pir_sensor_task.h>

static const char *TAG = "app_main";

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

// Definitions

#if CONFIG_ENABLE_ENCRYPTED_OTA
extern const char decryption_key_start[] asm("_binary_esp_image_encryption_key_pem_start");
extern const char decryption_key_end[] asm("_binary_esp_image_encryption_key_pem_end");

static const char *s_decryption_key = decryption_key_start;
static const uint16_t s_decryption_key_len = decryption_key_end - decryption_key_start;
#endif // CONFIG_ENABLE_ENCRYPTED_OTA

// Function declarations
static void occupancy_sensor_notification(uint16_t endpoint_id, bool occupancy, void *user_data);

extern "C" void app_main()
{

    esp_err_t err = ESP_OK;

    // Robust NVS init
    err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS needs erase (err=%d). Erasing and retrying...", err);
        nvs_flash_erase();
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_flash_init failed: %d", err);
        return;
    }

    // Create a Matter node and add the mandatory Root Node device type on endpoint 0
    node::config_t node_cfg;
    node_t *node = node::create(&node_cfg, app_attribute_update_cb, app_identification_cb);
    if (!node) {
        ESP_LOGE(TAG, "Failed to create Matter node");
        return;
    }

    // add the occupancy sensor
    occupancy_sensor::config_t occupancy_sensor_config;
    occupancy_sensor_config.occupancy_sensing.occupancy_sensor_type = chip::to_underlying(OccupancySensing::OccupancySensorTypeEnum::kPir);
    occupancy_sensor_config.occupancy_sensing.occupancy_sensor_type_bitmap = chip::to_underlying(OccupancySensing::OccupancySensorTypeBitmap::kPir);
    occupancy_sensor_config.occupancy_sensing.feature_flags = chip::to_underlying(OccupancySensing::Feature::kPassiveInfrared);

    endpoint_t *occupancy_sensor_ep = occupancy_sensor::create(node, &occupancy_sensor_config, ENDPOINT_FLAG_NONE, NULL);
    if (!occupancy_sensor_ep) {
        ESP_LOGE(TAG, "Failed to create temperature_sensor endpoint");
        return;
    }

    // Start Matter stack (this starts transports, commissioning, etc.)
    err = esp_matter::start(app_event_cb);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_matter::start failed: %d", err);
        return;
    }

    // initialize occupancy sensor driver (pir)
    static pir_sensor_config_t pir_config = {
        .cb = occupancy_sensor_notification,
        .endpoint_id = endpoint::get_id(occupancy_sensor_ep),
    };

    err = pir_sensor_init(&pir_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "pir_sensor_init failed: %d", err);
        return;
    }
}

static void occupancy_sensor_notification(uint16_t endpoint_id, bool occupancy, void *user_data)
{
    // schedule the attribute update so that we can report it from matter thread
    chip::DeviceLayer::SystemLayer().ScheduleLambda([endpoint_id, occupancy]() {
        attribute_t *attribute =
            attribute::get(endpoint_id, OccupancySensing::Id, OccupancySensing::Attributes::Occupancy::Id);

        esp_matter_attr_val_t val = esp_matter_invalid(NULL);
        attribute::get_val(attribute, &val);
        val.val.b = occupancy;

        attribute::update(endpoint_id, OccupancySensing::Id, OccupancySensing::Attributes::Occupancy::Id, &val);
    });
}