#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#include "http_server_task.h"
#include "pir_detection_task.h"

static const char *TAG = "pir_detection";

static TimerHandle_t detection_timer = NULL;

httpd_handle_t server;

// Forward declarations
static void IRAM_ATTR pir_isr_handler(void *arg);
static void detection_timer_callback(TimerHandle_t xTimer);

void pir_detection_task_init(void)
{
    // --- GPIO Configuration ---
    gpio_config_t pir_conf = {
        .pin_bit_mask = 1ULL << PIR_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE};
    gpio_config(&pir_conf);

    gpio_config_t led_conf = {
        .pin_bit_mask = 1ULL << LED_PIN,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&led_conf);

    gpio_set_level(LED_PIN, 0);

    // --- Timer Setup ---
    detection_timer = xTimerCreate(
        "DetectionTimer",
        pdMS_TO_TICKS(DETECTION_TIME_MS),
        pdFALSE,
        NULL,
        detection_timer_callback);

    // --- ISR Setup ---
    gpio_isr_handler_add(PIR_PIN, pir_isr_handler, NULL);

    ESP_LOGI(TAG, "PIR sensor initialized (PIR=%d, LED=%d)", PIR_PIN, LED_PIN);
}

// ISR — triggered on both edges
static void IRAM_ATTR pir_isr_handler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    int level = gpio_get_level(PIR_PIN);

    if (level == 1) {
        ESP_EARLY_LOGI(TAG, "Motion started");
        xTimerStartFromISR(detection_timer, &xHigherPriorityTaskWoken);
    } else {
        gpio_set_level(LED_PIN, 0);

        stop_webserver(server);

        ESP_EARLY_LOGI(TAG, "Motion ended");
        xTimerStopFromISR(detection_timer, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// Timer callback
static void detection_timer_callback(TimerHandle_t xTimer)
{
    if (gpio_get_level(PIR_PIN) == 1) {
        gpio_set_level(LED_PIN, 1);

        start_webserver(server);

        ESP_LOGI(TAG, "Motion sustained for 5 seconds!");
    }
}
