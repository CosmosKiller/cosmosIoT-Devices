#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/timers.h>
#include <lib/support/CodeUtils.h>

#include <evt_service_task.h>
#include <security_module_task.h>

using namespace chip::app::Clusters;
using namespace esp_matter;

static const char *TAG = "security_module_task";

static DRAM_ATTR TimerHandle_t detection_timer = NULL;
static DRAM_ATTR QueueHandle_t event_queue = NULL;

typedef struct {
    security_module_config_t *config;
    bool is_initialized;
} security_module_ctx_t;

static security_module_ctx_t s_ctx;

/**
 * @brief ISR handler for PIR sensor
 *
 * @param pArg
 */
static void IRAM_ATTR security_module_task_pir_isr_handler(void *pArg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    int level = gpio_get_level(PIR_PIN);

    // Call Matter callback if initialized
    if (s_ctx.is_initialized && s_ctx.config && s_ctx.config->pir_sensor.cb) {
        s_ctx.config->pir_sensor.cb(s_ctx.config->pir_sensor.endpoint_id, level, s_ctx.config->user_data);
    }

    // Post event to service
    evt_service_event_t evt = {
        .source = EVT_SOURCE_PIR,
        .type = level ? EVT_TYPE_TRIGGERED : EVT_TYPE_CLEARED,
        .value = level,
    };
    evt_service_post(&evt);

    if (level == 1) {
        xTimerStartFromISR(detection_timer, &xHigherPriorityTaskWoken);
    } else {
        xTimerStopFromISR(detection_timer, &xHigherPriorityTaskWoken);
    }

    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

/**
 * @brief Timer callback for PIR sensor sustained detection
 *
 * @param xTimer
 */
static void security_module_task_pir_timer_cb(TimerHandle_t xTimer)
{
    int level = gpio_get_level(PIR_PIN);

    if (level == 1) {
        // Post event to service
        evt_service_event_t evt = {
            .source = EVT_SOURCE_PIR,
            .type = EVT_TYPE_SUSTAINED,
            .value = level,
        };
        evt_service_post(&evt);
    }
}

esp_err_t security_module_task_init(security_module_config_t *pConfig)
{
    if (pConfig == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (s_ctx.is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    // PIR Sensor config
    gpio_config_t pir_conf = {
        .pin_bit_mask = (1ULL << PIR_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_ANYEDGE};
    gpio_config(&pir_conf);

    // Timer setup for trigger time
    detection_timer = xTimerCreate(
        "DetectionTimer",
        pdMS_TO_TICKS(TRIGGER_TIME_MS),
        pdFALSE,
        NULL,
        security_module_task_pir_timer_cb);

    gpio_isr_handler_add(PIR_PIN, security_module_task_pir_isr_handler, NULL);

    ESP_LOGI(TAG, "Security module initialized (PIR=%d, CONTACT=%d, PANIC=%d)", PIR_PIN, CONTACT_PIN, PANIC_PIN);

    s_ctx.config = pConfig;
    s_ctx.is_initialized = true;
    return ESP_OK;
}