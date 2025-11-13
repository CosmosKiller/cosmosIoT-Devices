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
#include <cam_task.h>
#include <http_server_task.h>
#include <matter_task.h>
#include <security_module_task.h>

static const char *TAG = "app_main";

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

// Definitions
uint16_t intercom_endpoint_id = 0;
httpd_handle_t cam_server;

// Function declarations
static void occupancy_sensor_notification(uint16_t endpoint_id, bool occupancy, void *user_data);
static void doorbell_notification(uint16_t endpoint_id, bool pressed, void *user_data);

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

    // Initialize GPIO ISR service
    err = gpio_install_isr_service(0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "GPIO ISR service install failed: %d", err);
        return;
    }

    // Create a Matter node and add the mandatory Root Node device type on endpoint 0
    node::config_t node_cfg;
    node_t *node = node::create(&node_cfg, app_attribute_update_cb, app_identification_cb);
    if (!node) {
        ESP_LOGE(TAG, "Failed to create Matter node");
        return;
    }

    // Add on-off plugin unit (for intercom camera)
    on_off_plug_in_unit::config_t intercom_config;
    intercom_config.on_off.on_off = false;

    endpoint_t *intercom_ep = on_off_plug_in_unit::create(node, &intercom_config, ENDPOINT_FLAG_NONE, NULL);
    if (!intercom_ep) {
        ESP_LOGE(TAG, "Failed to create cam toggle endpoint");
        return;
    }
    intercom_endpoint_id = endpoint::get_id(intercom_ep);

    // Add the occupancy sensor
    occupancy_sensor::config_t occupancy_sensor_config;
    occupancy_sensor_config.occupancy_sensing.occupancy_sensor_type = chip::to_underlying(OccupancySensing::OccupancySensorTypeEnum::kPir);
    occupancy_sensor_config.occupancy_sensing.occupancy_sensor_type_bitmap = chip::to_underlying(OccupancySensing::OccupancySensorTypeBitmap::kPir);
    occupancy_sensor_config.occupancy_sensing.feature_flags = chip::to_underlying(OccupancySensing::Feature::kPassiveInfrared);

    endpoint_t *occupancy_sensor_ep = occupancy_sensor::create(node, &occupancy_sensor_config, ENDPOINT_FLAG_NONE, NULL);
    if (!occupancy_sensor_ep) {
        ESP_LOGE(TAG, "Failed to create temperature_sensor endpoint");
        return;
    }

    // Add generic switch (for doorbell)
    generic_switch::config_t doorbell_config;
    doorbell_config.switch_cluster.number_of_positions = 2;
    doorbell_config.switch_cluster.current_position = 0;
    doorbell_config.switch_cluster.feature_flags = chip::to_underlying(Switch::Feature::kMomentarySwitch);

    endpoint_t *doorbell_ep = generic_switch::create(node, &doorbell_config, ENDPOINT_FLAG_NONE, NULL);
    if (!doorbell_ep) {
        ESP_LOGE(TAG, "Failed to create doorbell endpoint");
        return;
    }

    // Initialize camera driver
    err = cam_task_init();

    // Initialize occupancy sensor driver (pir)
    static security_module_config_t sec_mod_config = {
        .pir_sensor =
            {
                .cb = occupancy_sensor_notification,
                .endpoint_id = endpoint::get_id(occupancy_sensor_ep),
            },
        .doorbell =
            {
                .cb = doorbell_notification,
                .endpoint_id = endpoint::get_id(doorbell_ep),
            },
        .user_data = NULL,
    };

    // Initialize security module driver
    err = security_module_task_init(&sec_mod_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Security module init failed: %d", err);
        return;
    }

    // Start Matter stack (this starts transports, commissioning, etc.)
    err = esp_matter::start(app_event_cb);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_matter::start failed: %d", err);
        return;
    }

    // Start HTTP server task
    http_server_task_start(cam_server);
}

static void occupancy_sensor_notification(uint16_t endpoint_id, bool occupancy, void *user_data)
{
    // schedule the attribute update so that we can report it from matter thread
    chip::DeviceLayer::SystemLayer().ScheduleLambda([endpoint_id, occupancy]() {
        attribute_t *attribute =
            attribute::get(endpoint_id, Switch::Id, OccupancySensing::Attributes::Occupancy::Id);

        esp_matter_attr_val_t val = esp_matter_invalid(NULL);
        attribute::get_val(attribute, &val);
        val.val.b = occupancy;

        attribute::update(endpoint_id, Switch::Id, OccupancySensing::Attributes::Occupancy::Id, &val);
    });
}

static void doorbell_notification(uint16_t endpoint_id, bool pressed, void *user_data)
{
    // schedule the attribute update so that we can report it from matter thread
    chip::DeviceLayer::SystemLayer().ScheduleLambda([endpoint_id, pressed]() {
        attribute_t *attribute =
            attribute::get(endpoint_id, Switch::Id, Switch::Attributes::CurrentPosition::Id);

        esp_matter_attr_val_t val = esp_matter_invalid(NULL);
        attribute::get_val(attribute, &val);
        val.val.b = pressed;

        attribute::update(endpoint_id, Switch::Id, Switch::Attributes::CurrentPosition::Id, &val);
    });
}