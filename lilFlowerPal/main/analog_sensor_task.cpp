/**
 * @file analog_sensor_task.c
 * @author Marcel Nahir Samur (mnsamur2014@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-06-13
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <esp_err.h>
#include <esp_log.h>
#include <esp_random.h>
#include <esp_timer.h>

#include <lib/support/CodeUtils.h>

#include <analog_sensor_task.h>

static const char *TAG = "analog_sensor_task";

/**
 * @brief Context structure for the sensors
 *        Holds all the state and configuration needed for the driver.
 */
typedef struct {
    cosmos_sensor_t *sn_param;  /*!< Sensors paramters array*/
    an_sensor_config_t *config; /*!< Sensor configurations array*/
    esp_timer_handle_t timer;
    bool is_initialized = false;
} an_sensor_ctx_t;

static an_sensor_ctx_t s_ctx;

/**
 * @brief Trigger sensor readings
 *
 * @param pvParameters Parameter which can be passed to the task.
 */
static void analog_sensor_task_read_cb(void *pArg)
{
    float current_reading;

    auto *ctx = (an_sensor_ctx_t *)pArg;
    if (!(ctx && ctx->config)) {
        return;
    }

    cosmos_sensor_adc_read_raw(ctx->sn_param, SNR_QTY);

    for (size_t i = 0; i < SNR_QTY; i++) {

        switch (ctx->sn_param[i].snr_type) {
        case SNR_TYPE_WL:
            current_reading = ctx->sn_param[i].reading;
            break;

        case SNR_TYPE_SM:
            current_reading = COSMOS_MAP(ctx->sn_param[i].reading, 3000, 1500, 0, 100);
            break;

        default:
            ESP_LOGE(TAG, "Sensor type %d not supported", ctx->sn_param[i].snr_type);
            break;
        }

        if (ctx->config[i].cb) {
            ctx->config[i].cb(ctx->config[i].endpoint_id, current_reading, ctx->config[i].user_data);
        }

        /*
        As the soil gets wetter, the output value decreases, and as it gets drier,
        the output value increases. When powered at 5V, the output ranges from
        about 1.5V (for wet soil) to 3V (for dry soil).

        Source: https://lastminuteengineers.com/capacitive-soil-moisture-sensor-arduino/
        */
        ESP_LOGI(TAG, "Moisture sensor endpoint %d: %d\n", ctx->config[i].endpoint_id, ctx->sn_param[i].reading);
    }
}

esp_err_t analog_sensor_task_sensor_init(an_sensor_config_t *pConfig, cosmos_sensor_t *pSensor)
{
    esp_err_t err;

    if (pConfig == NULL || pSensor == NULL) {
        ESP_LOGE(TAG, "Invalid argument");
        return ESP_ERR_INVALID_ARG;
    }
    if (pConfig->cb == NULL) {
        ESP_LOGE(TAG, "Callback function must be provided");
        return ESP_ERR_INVALID_ARG;
    }
    if (s_ctx.is_initialized) {
        ESP_LOGE(TAG, "Driver already initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Keep the configuration and sensor parameters
    s_ctx.config = pConfig;
    s_ctx.sn_param = pSensor;

    // Create a periodic timer to read the sensor
    const esp_timer_create_args_t read_args = {
        .callback = analog_sensor_task_read_cb,
        .arg = &s_ctx,
        .name = "analog_sensor_read",
    };

    err = esp_timer_create(&read_args, &s_ctx.timer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create timer for soil moisture sensor");
        return err;
    }

    // Start the timer to trigger every 5 seconds
    err = esp_timer_start_periodic(s_ctx.timer, pConfig->interval_ms * 1000);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start timer for soil moisture sensor");
        esp_timer_delete(s_ctx.timer);
        return err;
    }

    s_ctx.is_initialized = true;

    return ESP_OK;
}