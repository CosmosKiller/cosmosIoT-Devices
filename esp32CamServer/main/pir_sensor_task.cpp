#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include <esp_log.h>
#include <lib/support/CodeUtils.h>

#include <pir_sensor_task.h>

static const char *TAG = "pir_sensor_task";

static TimerHandle_t detection_timer = NULL;

typedef struct {
    pir_sensor_config_t *config;
    bool is_initialized;
} pir_sensor_ctx_t;

static pir_sensor_ctx_t s_ctx;

static void IRAM_ATTR pir_sensor_isr_handler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    int level = gpio_get_level(PIR_PIN);

    if (level == 1) {
        // Motion detected
        ESP_EARLY_LOGI(TAG, "Motion started");
        xTimerStartFromISR(detection_timer, &xHigherPriorityTaskWoken);
    } else {
        // No motion
        gpio_set_level(LED_PIN, 0);

        if (s_ctx.config->cb) {
            s_ctx.config->cb(s_ctx.config->endpoint_id, level, s_ctx.config->user_data);
        }

        ESP_EARLY_LOGI(TAG, "Motion ended");
        xTimerStopFromISR(detection_timer, &xHigherPriorityTaskWoken);
    }
}

// Timer callback
static void pir_sensor_timer_cb(TimerHandle_t xTimer)
{
    int level = gpio_get_level(PIR_PIN);

    if (level == 1) {
        gpio_set_level(LED_PIN, 1);

        if (s_ctx.config->cb) {
            s_ctx.config->cb(s_ctx.config->endpoint_id, level, s_ctx.config->user_data);
        }
        ESP_LOGI(TAG, "Motion sustained for 5 seconds!");
    }
}

esp_err_t pir_sensor_init(pir_sensor_config_t *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (s_ctx.is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    // LED config
    gpio_config_t led_conf = {
        .pin_bit_mask = (1ULL << LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&led_conf);

    // PIR Sensor config
    gpio_config_t pir_conf = {
        .pin_bit_mask = (1ULL << PIR_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_ANYEDGE};
    gpio_config(&pir_conf);

    gpio_set_level(LED_PIN, 0);

    // Timer setup for trigger time
    detection_timer = xTimerCreate(
        "DetectionTimer",
        pdMS_TO_TICKS(TRIGGER_TIME_MS),
        pdFALSE,
        NULL,
        pir_sensor_timer_cb);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(PIR_PIN, pir_sensor_isr_handler, NULL);

    ESP_LOGI(TAG, "PIR sensor initialized (PIR=%d, LED=%d)", PIR_PIN, LED_PIN);

    s_ctx.config = config;
    return ESP_OK;
}