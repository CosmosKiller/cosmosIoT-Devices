#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/timers.h>
#include <lib/support/CodeUtils.h>

#include <http_stream_task.h>
#include <security_module_task.h>

using namespace chip::app::Clusters;
using namespace esp_matter;

static const char *TAG = "security_module_task";
extern uint16_t intercom_endpoint_id;

static DRAM_ATTR TimerHandle_t detection_timer = NULL;
static DRAM_ATTR QueueHandle_t event_queue = NULL;

typedef struct {
    security_module_config_t *config;
    bool is_initialized;
} security_module_ctx_t;

static security_module_ctx_t s_ctx;

/**
 * @brief Handle security module events
 *
 * @param pArg Arguments that can be passed to the task
 * @return esp_err_t
 */
static void security_module_task_evt_handle(void *pArg)
{
    security_event_t evt;

    while (1) {
        if (xQueueReceive(event_queue, &evt, portMAX_DELAY)) {
            switch (evt.type) {
            case EVENT_MOTION_START:
                ESP_LOGI(TAG, "Motion detected!");
                break;

            case EVENT_MOTION_SUSTAINED:
                ESP_LOGI(TAG, "Motion sustained... Starting webserver");
                gpio_set_level(LED_PIN, 1);
                http_stream_task_service_enabled(true);
                break;

            case EVENT_CALL_REQUEST:
                ESP_LOGI(TAG, "Doorbell pressed! Call requested.");
                gpio_set_level(LED_PIN, 1);
                http_stream_task_service_enabled(true);
                break;

            case EVENT_CALL_START:
                ESP_LOGI(TAG, "Call started!");
                gpio_set_level(LED_PIN, 1);
                http_stream_task_service_enabled(true);
                break;

            case EVENT_ALL_END:
                ESP_LOGI(TAG, "Security event ended.");
                gpio_set_level(LED_PIN, 0);
                http_stream_task_service_enabled(false);
                break;

            default:
                ESP_LOGW(TAG, "Unknown event type");
                break;
            }
        }
    }
}

/**
 * @brief ISR handler for PIR sensor
 *
 * @param pArg
 */
static void IRAM_ATTR security_module_task_pir_isr_handler(void *pArg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    int level = gpio_get_level(PIR_PIN);

    if (s_ctx.config->pir_sensor.cb) {
        s_ctx.config->pir_sensor.cb(s_ctx.config->pir_sensor.endpoint_id, level, s_ctx.config->user_data);
    }

    security_event_t evt;
    if (level == 1) {
        evt.type = EVENT_MOTION_START;
        xQueueSendFromISR(event_queue, &evt, &xHigherPriorityTaskWoken);
        xTimerStartFromISR(detection_timer, &xHigherPriorityTaskWoken);
    } else {
        evt.type = EVENT_ALL_END;
        xQueueSendFromISR(event_queue, &evt, &xHigherPriorityTaskWoken);
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
        security_event_t evt = {
            .type = EVENT_MOTION_SUSTAINED,
            .level = level,
        };
        xQueueSend(event_queue, &evt, 0);
    }
}

/**
 * @brief ISR handler for doorbell button
 *
 * @param pArg
 */
static void IRAM_ATTR security_module_task_doorbell_isr_handler(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    int level = gpio_get_level(DOORBELL_PIN);

    if (s_ctx.config->doorbell.cb) {
        s_ctx.config->doorbell.cb(s_ctx.config->doorbell.endpoint_id, level, s_ctx.config->user_data);
    }

    if (level == 1) {
        security_event_t evt = {
            .type = EVENT_CALL_REQUEST,
            .level = level,
        };
        xQueueSendFromISR(event_queue, &evt, &xHigherPriorityTaskWoken);
    }

    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

/**
 * @brief Handle from intercom attribute updates
 *
 * @param level Current intercom level
 */
static void security_module_task_intercom_handle(esp_matter_attr_val_t *val)
{
    bool level = val->val.b;

    if (level) {
        security_event_t evt = {
            .type = EVENT_CALL_START,
            .level = level,
        };
        xQueueSend(event_queue, &evt, 0);
    } else {
        security_event_t evt = {
            .type = EVENT_ALL_END,
            .level = level,
        };
        xQueueSend(event_queue, &evt, 0);
    }
}

esp_err_t security_module_attribute_update(security_module_task_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                           uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    esp_err_t err = ESP_OK;

    if (endpoint_id == intercom_endpoint_id) {
        if (cluster_id == OnOff::Id) {
            if (attribute_id == OnOff::Attributes::OnOff::Id) {
                security_module_task_intercom_handle(val);
            }
        }
    }
    return err;
}

esp_err_t security_module_task_init(security_module_config_t *pConfig)
{
    if (pConfig == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (s_ctx.is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    // Create event queue before enabling interrupts
    event_queue = xQueueCreate(20, sizeof(security_event_t));
    if (!event_queue) {
        return ESP_ERR_NO_MEM;
    }

    // Create event handling task
    BaseType_t ret = xTaskCreatePinnedToCore(
        security_module_task_evt_handle,
        "security_module_task_evt_handle",
        3072,
        NULL,
        5,
        NULL,
        0);
    if (ret != pdPASS) {
        vQueueDelete(event_queue);
        return ESP_ERR_NO_MEM;
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

    // Doorbell button config
    gpio_config_t doorbell_conf = {
        .pin_bit_mask = (1ULL << DOORBELL_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_ANYEDGE};
    gpio_config(&doorbell_conf);

    gpio_set_level(LED_PIN, 0);

    // Timer setup for trigger time
    detection_timer = xTimerCreate(
        "DetectionTimer",
        pdMS_TO_TICKS(TRIGGER_TIME_MS),
        pdFALSE,
        NULL,
        security_module_task_pir_timer_cb);

    if (!detection_timer) {
        vQueueDelete(event_queue);
        return ESP_ERR_NO_MEM;
    }

    gpio_isr_handler_add(PIR_PIN, security_module_task_pir_isr_handler, NULL);
    gpio_isr_handler_add(DOORBELL_PIN, security_module_task_doorbell_isr_handler, NULL);

    ESP_LOGI(TAG, "Security module initialized (PIR=%d, LED=%d, DORBELL=%d)", PIR_PIN, LED_PIN, DOORBELL_PIN);

    s_ctx.config = pConfig;
    s_ctx.is_initialized = true;
    return ESP_OK;
}