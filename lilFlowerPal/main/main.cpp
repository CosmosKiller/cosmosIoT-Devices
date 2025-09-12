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

#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <esp_matter.h>
#include <esp_matter_console.h>
#include <esp_matter_ota.h>

#include <main_tasks_common.h>
#include <pump_task.h>
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/ESP32/OpenthreadLauncher.h>
#endif

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>

static const char *TAG = "app_main";

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

uint16_t configure_pumps = 0;
pump_task_config_t pump_data[4] = {};
gpio_pump_t pump_gpios[4] = {
    {.GPIO_PIN_VALUE = PUMP0},
    {.GPIO_PIN_VALUE = PUMP1},
    {.GPIO_PIN_VALUE = PUMP2},
    {.GPIO_PIN_VALUE = PUMP3},
};

constexpr auto k_timeout_seconds = 300;

#if CONFIG_ENABLE_ENCRYPTED_OTA
extern const char decryption_key_start[] asm("_binary_esp_image_encryption_key_pem_start");
extern const char decryption_key_end[] asm("_binary_esp_image_encryption_key_pem_end");

static const char *s_decryption_key = decryption_key_start;
static const uint16_t s_decryption_key_len = decryption_key_end - decryption_key_start;
#endif // CONFIG_ENABLE_ENCRYPTED_OTA

static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
    switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
        ESP_LOGI(TAG, "Interface IP Address changed");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
        ESP_LOGI(TAG, "Commissioning complete");
        break;

    case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
        ESP_LOGI(TAG, "Commissioning failed, fail safe timer expired");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStarted:
        ESP_LOGI(TAG, "Commissioning session started");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStopped:
        ESP_LOGI(TAG, "Commissioning session stopped");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened:
        ESP_LOGI(TAG, "Commissioning window opened");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed:
        ESP_LOGI(TAG, "Commissioning window closed");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricRemoved: {
        ESP_LOGI(TAG, "Fabric removed successfully");
        if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
            chip::CommissioningWindowManager &commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
            constexpr auto kTimeoutSeconds = chip::System::Clock::Seconds16(k_timeout_seconds);
            if (!commissionMgr.IsCommissioningWindowOpen()) {
                /* After removing last fabric, this example does not remove the Wi-Fi credentials
                 * and still has IP connectivity so, only advertising on DNS-SD.
                 */
                CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds,
                                                                            chip::CommissioningWindowAdvertisement::kDnssdOnly);
                if (err != CHIP_NO_ERROR) {
                    ESP_LOGE(TAG, "Failed to open commissioning window, err:%" CHIP_ERROR_FORMAT, err.Format());
                }
            }
        }
        break;
    }

    case chip::DeviceLayer::DeviceEventType::kFabricWillBeRemoved:
        ESP_LOGI(TAG, "Fabric will be removed");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricUpdated:
        ESP_LOGI(TAG, "Fabric is updated");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricCommitted:
        ESP_LOGI(TAG, "Fabric is committed");
        break;

    case chip::DeviceLayer::DeviceEventType::kBLEDeinitialized:
        ESP_LOGI(TAG, "BLE deinitialized and memory reclaimed");
        break;

    default:
        break;
    }
}

// This callback is invoked when clients interact with the Identify Cluster.
// In the callback implementation, an endpoint can identify itself. (e.g., by flashing an LED or light).
static esp_err_t app_identification_cb(identification::callback_type_t type, uint16_t endpoint_id, uint8_t effect_id,
                                       uint8_t effect_variant, void *priv_data)
{
    ESP_LOGI(TAG, "Identification callback: type: %u, effect: %u, variant: %u", type, effect_id, effect_variant);
    return ESP_OK;
}

// This callback is called for every attribute update. The callback implementation shall
// handle the desired attributes and return an appropriate error code. If the attribute
// is not of your interest, please do not return an error code and strictly return ESP_OK.
static esp_err_t app_attribute_update_cb(attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id,
                                         uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data)
{
    esp_err_t err = ESP_OK;

    if (type == PRE_UPDATE) {
        /* Driver update */
        pump_task_handle_t pump_handle = (pump_task_handle_t)priv_data;
        err = pump_task_attribute_update(pump_handle, endpoint_id, cluster_id, attribute_id, val);
    }

    return err;
}

// Creates pump-endpoint mapping for each GPIO pin configured.
static esp_err_t app_create_pump(gpio_pump_t *pump, node_t *node)
{
    esp_err_t err = ESP_OK;

    if (!node) {
        ESP_LOGE(TAG, "Matter node cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }

    if (!pump) {
        ESP_LOGE(TAG, "Plug cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }

    /* Create the pump endpoint */
    on_off_plugin_unit::config_t pump_config;
    pump_config.on_off.on_off = DEFAULT_POWER;
    endpoint_t *endpoint = on_off_plugin_unit::create(node, &pump_config, ENDPOINT_FLAG_NONE, pump);

    /* Confirm that node and endpoint were created successfully */
    if (!endpoint) {
        ESP_LOGE(TAG, "Matter node creation failed");
        return ESP_FAIL;
    }

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize plug");
    }

    // Check for maximum plugs that can be configured.
    if (configure_pumps < 4) {
        pump_data[configure_pumps].gpio = pump->GPIO_PIN_VALUE;
        pump_data[configure_pumps].endpoint_id = endpoint::get_id(endpoint);
        configure_pumps++;
    } else {
        ESP_LOGE(TAG, "Maximum plugs configuration limit exceeded!!!");
        return ESP_FAIL;
    }

    /* Get Endpoints Id */
    uint16_t pump_endpoint_id = endpoint::get_id(endpoint);
    ESP_LOGI(TAG, "Pump %d created with endpoint_id %d", configure_pumps, pump_endpoint_id);

    return err;
}

// Task to initialize pump tasks for each configured pump.
static void app_init_pump(void *arg)
{
    for (int i = 0; i < configure_pumps; ++i) {
        esp_err_t r = pump_task_init(&pump_gpios[i]);
        if (r != ESP_OK) {
            ESP_LOGE("pump", "pump_task_init failed for pump %d", i);
        }
        vTaskDelay(pdMS_TO_TICKS(50)); // small delay between inits
    }
    vTaskDelete(NULL);
}

// Start the pump initialization task
static void app_start_pump(void)
{
    xTaskCreatePinnedToCore(app_init_pump, "app_init_pump", PUMP_INIT_TASK_STACK_SIZE, NULL,
                            PUMP_INIT_TASK_PRIORITY, NULL, PUMP_INIT_TASK_CORE_ID);
}

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

    // Init event loop and netif (Matter expects these)
    /*         err = esp_event_loop_create_default();
            if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
                ESP_LOGE(TAG, "esp_event_loop_create_default failed: %d", err);
                return;
            } */

    // Create node (check result)
    node::config_t node_cfg;
    node_t *node = node::create(&node_cfg, app_attribute_update_cb, app_identification_cb);
    if (!node) {
        ESP_LOGE(TAG, "Failed to create Matter node");
        return;
    }

    app_create_pump(&pump_gpios[0], node);
    app_create_pump(&pump_gpios[1], node);
    app_create_pump(&pump_gpios[2], node);
    app_create_pump(&pump_gpios[3], node);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    /* Set OpenThread platform config */
    esp_openthread_platform_config_t config = {
        .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
        .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };
    set_openthread_platform_config(&config);
#endif

    // Start Matter stack (this starts transports, commissioning, etc.)
    err = esp_matter::start(app_event_cb);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_matter::start failed: %d", err);
        return;
    }

    app_start_pump();

#if CONFIG_ENABLE_ENCRYPTED_OTA
    err = esp_matter_ota_requestor_encrypted_init(s_decryption_key, s_decryption_key_len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialized the encrypted OTA, err: %d", err);
    }
#endif // CONFIG_ENABLE_ENCRYPTED_OTA

#if CONFIG_ENABLE_CHIP_SHELL
    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::wifi_register_commands();
    esp_matter::console::factoryreset_register_commands();
#if CONFIG_OPENTHREAD_CLI
    esp_matter::console::otcli_register_commands();
#endif
    esp_matter::console::init();
#endif
}