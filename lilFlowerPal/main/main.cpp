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

// Include ESP-MATTER libraries
#include <esp_matter.h>
#include <esp_matter_console.h>
#include <esp_matter_ota.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/ESP32/OpenthreadLauncher.h>
#endif

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>

// Include project libraries
#include <bme680_task.h>
#include <main_tasks_common.h>
#include <matter_task.h>
#include <pump_task.h>
#include <soil_moisture_task.h>

static const char *TAG = "app_main";

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

// Pump definitions
uint16_t pumps_qty = 0;
pump_task_config_t pumps_config[4] = {};
gpio_pump_t pump_gpios[4] = {
    {.GPIO_PIN_VALUE = PUMP0},
    {.GPIO_PIN_VALUE = PUMP1},
    {.GPIO_PIN_VALUE = PUMP2},
    {.GPIO_PIN_VALUE = PUMP3},
};

// Soil moisture sensor definitions
uint16_t sm_sensor_qty = 0;
sm_sensor_config_t sm_sensors_config[4] = {};
cosmos_sensor_t sm_sensors[4] = {
    {.pin_num = SN1_GPIO, .snr_chn = SN1_CHN},
    {.pin_num = SN2_GPIO, .snr_chn = SN2_CHN},
    {.pin_num = SN3_GPIO, .snr_chn = SN3_CHN},
    {.pin_num = SN4_GPIO, .snr_chn = SN4_CHN},
};

#if CONFIG_ENABLE_ENCRYPTED_OTA
extern const char decryption_key_start[] asm("_binary_esp_image_encryption_key_pem_start");
extern const char decryption_key_end[] asm("_binary_esp_image_encryption_key_pem_end");

static const char *s_decryption_key = decryption_key_start;
static const uint16_t s_decryption_key_len = decryption_key_end - decryption_key_start;
#endif // CONFIG_ENABLE_ENCRYPTED_OTA

// Function declarations
static esp_err_t app_create_pump(gpio_pump_t *pPump, node_t *pNode);
static void app_init_pump(void *pvParameters);
static esp_err_t app_create_sm_sensor(sm_sensor_config_t *pSensor, node_t *pNode);
static void app_init_sm_sensor(void *pvParameters);
static void temp_sensor_notification(uint16_t endpoint_id, float temp, void *user_data);
static void humidity_sensor_notification(uint16_t endpoint_id, float humidity, void *user_data);
static void pressure_sensor_notification(uint16_t endpoint_id, float pressure, void *user_data);

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

    // Add temperature sensor device
    temperature_sensor::config_t temp_sensor_config;
    endpoint_t *temp_sensor_ep = temperature_sensor::create(node, &temp_sensor_config, ENDPOINT_FLAG_NONE, NULL);
    // Confirm that temperature_sensor endpoint was created successfully
    if (!temp_sensor_ep) {
        ESP_LOGE(TAG, "Failed to create temperature_sensor endpoint");
        return;
    }

    // Add the humidity sensor device
    humidity_sensor::config_t humidity_sensor_config;
    endpoint_t *humidity_sensor_ep = humidity_sensor::create(node, &humidity_sensor_config, ENDPOINT_FLAG_NONE, NULL);
    // Confirm that humidity_sensor endpoint was created successfully
    if (!temp_sensor_ep) {
        ESP_LOGE(TAG, "Failed to create humidity_sensor endpoint");
        return;
    }

    // Add the pressure sensor device
    pressure_sensor::config_t pressure_sensor_config;
    endpoint_t *pressure_sensor_ep = pressure_sensor::create(node, &pressure_sensor_config, ENDPOINT_FLAG_NONE, NULL);
    // Confirm that pressure_sensor endpoint was created successfully
    if (!pressure_sensor_ep) {
        ESP_LOGE(TAG, "Failed to create pressure_sensor endpoint");
        return;
    }

    // Initialize BME680 sensor task
    static bme680_sensor_config_t bme680_sensor_config = {
        .temperature =
            {
                .cb = temp_sensor_notification,
                .endpoint_id = endpoint::get_id(temp_sensor_ep),
            },
        .humidity =
            {
                .cb = humidity_sensor_notification,
                .endpoint_id = endpoint::get_id(humidity_sensor_ep),
            },
        .pressure =
            {
                .cb = pressure_sensor_notification,
                .endpoint_id = endpoint::get_id(pressure_sensor_ep),
            },
    };

    // Create soil moisture sensor endpoints for each configured GPIO pin
    app_create_sm_sensor(&sm_sensors_config[0], node);
    sm_sensors_config[0].cb = humidity_sensor_notification;

    app_create_sm_sensor(&sm_sensors_config[1], node);
    sm_sensors_config[1].cb = humidity_sensor_notification;

    app_create_sm_sensor(&sm_sensors_config[2], node);
    sm_sensors_config[2].cb = humidity_sensor_notification;

    app_create_sm_sensor(&sm_sensors_config[3], node);
    sm_sensors_config[3].cb = humidity_sensor_notification;

    // Create pump endpoints for each configured GPIO pin
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

    // Initialize BME680 sensor task
    err = bme680_task_sensor_init(&bme680_sensor_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "bme680_task_sensor_init failed: %d", err);
        return;
    }

    // Start the soil moisture sensor tasks
    xTaskCreatePinnedToCore(app_init_sm_sensor, "app_init_sm_sensor", SOIL_MOISTURE_TASK_STACK_SIZE, NULL,
                            SOIL_MOISTURE_TASK_PRIORITY, NULL, SOIL_MOISTURE_TASK_CORE_ID);

    // Start the pump tasks
    xTaskCreatePinnedToCore(app_init_pump, "app_init_pump", PUMP_TASK_STACK_SIZE, NULL,
                            PUMP_TASK_PRIORITY, NULL, PUMP_TASK_CORE_ID);

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

/*!
 * PUMP FUNCTIONS DEFINITIONS
 */

// Creates pump-endpoint mapping for each GPIO pin configured.
static esp_err_t app_create_pump(gpio_pump_t *pPump, node_t *pNode)
{
    esp_err_t err = ESP_OK;

    if (!pNode) {
        ESP_LOGE(TAG, "Matter node cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }

    if (!pPump) {
        ESP_LOGE(TAG, "Plug cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }

    // Create the pump endpoint
    on_off_plugin_unit::config_t pump_config;
    pump_config.on_off.on_off = DEFAULT_POWER;
    endpoint_t *endpoint = on_off_plugin_unit::create(pNode, &pump_config, ENDPOINT_FLAG_NONE, pPump);

    // Confirm that node and endpoint were created successfully
    if (!endpoint) {
        ESP_LOGE(TAG, "Failed to create plug endpoint");
        return ESP_FAIL;
    }

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize plug");
    }

    // Check for maximum plugs that can be configured.
    if (pumps_qty < 4) {
        pumps_config[pumps_qty].gpio = pPump->GPIO_PIN_VALUE;
        pumps_config[pumps_qty].endpoint_id = endpoint::get_id(endpoint);
        pumps_qty++;
    } else {
        ESP_LOGE(TAG, "Maximum plugs configuration limit exceeded!!!");
        return ESP_FAIL;
    }

    // Get Endpoints Id
    uint16_t pump_endpoint_id = endpoint::get_id(endpoint);
    ESP_LOGI(TAG, "Pump %d created with endpoint_id %d", pumps_qty, pump_endpoint_id);

    return err;
}

// Cycles through each configured pump and initializes it.
static void app_init_pump(void *pvParameters)
{
    for (int i = 0; i < pumps_qty; ++i) {
        esp_err_t r = pump_task_init(&pump_gpios[i]);
        if (r != ESP_OK) {
            ESP_LOGE(TAG, "pump_task_init failed for pump %d", i);
        }
        vTaskDelay(pdMS_TO_TICKS(50)); // small delay between inits
    }
    vTaskDelete(NULL);
}

/*!
 *  SOIL MOISTURE FUNCTIONS DEFINITIONS
 */

// Creates soil moisture endpoint mapping for each GPIO pin configured.
static esp_err_t app_create_sm_sensor(sm_sensor_config_t *pSensor, node_t *pNode)
{
    esp_err_t err = ESP_OK;

    if (!pNode) {
        ESP_LOGE(TAG, "Matter node cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }

    if (!pSensor) {
        ESP_LOGE(TAG, "Sensor cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }

    // Create the soil moisture endpoint
    humidity_sensor::config_t sm_sensor_config;
    endpoint_t *endpoint = humidity_sensor::create(pNode, &sm_sensor_config, ENDPOINT_FLAG_NONE, NULL);

    // Confirm that node and endpoint were created successfully
    if (!endpoint) {
        ESP_LOGE(TAG, "Failed to create plug endpoint");
        return ESP_FAIL;
    }

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize plug");
    }

    // Check for maximum plugs that can be configured.
    if (sm_sensor_qty < 4) {
        sm_sensors_config[sm_sensor_qty].endpoint_id = endpoint::get_id(endpoint);
        sm_sensor_qty++;
    } else {
        ESP_LOGE(TAG, "Maximum plugs configuration limit exceeded!!!");
        return ESP_FAIL;
    }

    // Get Endpoints Id
    uint16_t sm_sensor_endpoint_id = endpoint::get_id(endpoint);
    ESP_LOGI(TAG, "Pump %d created with endpoint_id %d", sm_sensor_qty, sm_sensor_endpoint_id);

    return err;
}

// Cycles through each configured soil moisture sensor and initializes it.
static void app_init_sm_sensor(void *pvParameters)
{
    for (int i = 0; i < sm_sensor_qty; ++i) {
        esp_err_t r = soil_moisture_task_sensor_init(&sm_sensors_config[i], &sm_sensors[i]);
        if (r != ESP_OK) {
            ESP_LOGE(TAG, "soil_moisture_task_sensor_init %d", i);
            vTaskDelay(pdMS_TO_TICKS(50)); // small delay between inits
        }
        vTaskDelete(NULL);
    }
}

/*
 * Application cluster specification, 2.3.4.1. Temperature
 * represents a temperature on the Celsius scale with a resolution of 0.01°C.
 * temp = (temperature in °C) x 100
 */
static void temp_sensor_notification(uint16_t endpoint_id, float temp, void *user_data)
{
    // schedule the attribute update so that we can report it from matter thread
    chip::DeviceLayer::SystemLayer().ScheduleLambda([endpoint_id, temp]() {
        attribute_t *attribute = attribute::get(endpoint_id,
                                                TemperatureMeasurement::Id,
                                                TemperatureMeasurement::Attributes::MeasuredValue::Id);

        esp_matter_attr_val_t val = esp_matter_invalid(NULL);
        attribute::get_val(attribute, &val);
        val.val.i16 = static_cast<int16_t>(temp * 100);

        attribute::update(endpoint_id, TemperatureMeasurement::Id, TemperatureMeasurement::Attributes::MeasuredValue::Id, &val);
    });
}

/*
 * Application cluster specification, 2.6.4.1. Relative Humidity
 * represents the humidity in percent.
 * humidity = (humidity in %) x 100
 *
 */
static void humidity_sensor_notification(uint16_t endpoint_id, float humidity, void *user_data)
{
    // schedule the attribute update so that we can report it from matter thread
    chip::DeviceLayer::SystemLayer().ScheduleLambda([endpoint_id, humidity]() {
        attribute_t *attribute = attribute::get(endpoint_id,
                                                RelativeHumidityMeasurement::Id,
                                                RelativeHumidityMeasurement::Attributes::MeasuredValue::Id);

        esp_matter_attr_val_t val = esp_matter_invalid(NULL);
        attribute::get_val(attribute, &val);
        val.val.u16 = static_cast<uint16_t>(humidity * 100);

        attribute::update(endpoint_id, RelativeHumidityMeasurement::Id, RelativeHumidityMeasurement::Attributes::MeasuredValue::Id, &val);
    });
}

/*
 * Application cluster specification, 2.4.5.1. Pressure
 * represents the pressure in Kilopascals (kPa).
 * pressure = (pressure in kPa) x 10
 *
 */
static void pressure_sensor_notification(uint16_t endpoint_id, float pressure, void *user_data)
{
    // schedule the attribute update so that we can report it from matter thread
    chip::DeviceLayer::SystemLayer().ScheduleLambda([endpoint_id, pressure]() {
        attribute_t *attribute = attribute::get(endpoint_id,
                                                PressureMeasurement::Id,
                                                PressureMeasurement::Attributes::MeasuredValue::Id);

        esp_matter_attr_val_t val = esp_matter_invalid(NULL);
        attribute::get_val(attribute, &val);
        val.val.u32 = static_cast<uint32_t>(pressure);

        attribute::update(endpoint_id, PressureMeasurement::Id, PressureMeasurement::Attributes::MeasuredValue::Id, &val);
    });
}

/* static void gas_sensor_notification(uint16_t endpoint_id, float gas_resistance, void *user_data)
{
    // schedule the attribute update so that we can report it from matter thread
    chip::DeviceLayer::SystemLayer().ScheduleLambda([endpoint_id, gas_resistance]() {
        attribute_t *attribute = attribute::get(endpoint_id,
                                                TotalVolatileOrganicCompoundsConcentrationMeasurement::Id,
                                                TotalVolatileOrganicCompoundsConcentrationMeasurement::Attributes::MeasuredValue::Id);

        esp_matter_attr_val_t val = esp_matter_invalid(NULL);
        attribute::get_val(attribute, &val);
        val.val.u32 = gas_resistance;

        attribute::update(endpoint_id, 0x042E, 0x0000, &val);
    });
} */