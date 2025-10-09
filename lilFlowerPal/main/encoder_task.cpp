/**
 * @file encoder_task.cpp
 * @author Marcel Nahir Samur (mnsamur2014@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-10-05
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <encoder_task.h>
#include <esp_attr.h>
#include <esp_log.h>

static const char *TAG = "encoder_task";

static volatile int last_AB = 0;
static volatile int new_AB = 0;

// Button ISR handler with debounce
static void IRAM_ATTR button_isr_handler(void *arg)
{
    encoder_config_t *encoder = (encoder_config_t *)arg;

    static uint32_t last_button_time = 0;
    uint32_t current_time = xTaskGetTickCountFromISR();

    // Simple debounce - ignore presses within 50ms
    if ((current_time - last_button_time) >= pdMS_TO_TICKS(50)) {
        encoder_config_t *encoder = (encoder_config_t *)arg;

        // Send event to queue instead of calling callback directly
        if (encoder->event_queue) {
            encoder_event_t evt = {
                .type = ENCODER_EVENT_BUTTON,
                .pressed = true,
            };
            xQueueSendFromISR(encoder->event_queue, &evt, NULL);
        }

        last_button_time = current_time;
    }
}

// Encoder ISR handler with acceleration and bounds
static void IRAM_ATTR encoder_isr_handler(void *arg)
{
    encoder_config_t *encoder = (encoder_config_t *)arg;

    // Get current time for debounce
    uint32_t now = xTaskGetTickCountFromISR();

    // Software debounce
    if ((now - encoder->last_change_time) < pdMS_TO_TICKS(ENCODER_DEBOUNCE_MS)) {
        return;
    }

    // Read current state
    int A = gpio_get_level(encoder->pin_a);
    int B = gpio_get_level(encoder->pin_b);

    // Get new state
    int new_AB = (A << 1) | B;
    int old_AB = encoder->last_AB;

    if (new_AB != old_AB) {
        // Store time for debounce
        encoder->last_change_time = now;

        // Check valid gray-code sequence
        if ((old_AB == 0b00 && new_AB == 0b01) ||
            (old_AB == 0b01 && new_AB == 0b11) ||
            (old_AB == 0b11 && new_AB == 0b10) ||
            (old_AB == 0b10 && new_AB == 0b00)) {
            encoder->counter--;
        } else if ((old_AB == 0b00 && new_AB == 0b10) ||
                   (old_AB == 0b10 && new_AB == 0b11) ||
                   (old_AB == 0b11 && new_AB == 0b01) ||
                   (old_AB == 0b01 && new_AB == 0b00)) {
            encoder->counter++;
        }

        if (encoder->event_queue) {
            encoder_event_t evt = {
                .type = ENCODER_EVENT_COUNT,
                .count = encoder->counter,
            };
            xQueueSendFromISR(encoder->event_queue, &evt, NULL);
        }

        encoder->last_AB = new_AB;
    }
}

esp_err_t encoder_init(encoder_config_t *encoder)
{
    if (!encoder) {
        return ESP_ERR_INVALID_ARG;
    }

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << encoder->pin_a) |
                        (1ULL << encoder->pin_b) |
                        (1ULL << encoder->pin_btn),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };

    esp_err_t err;
    if ((err = gpio_config(&io_conf)) != ESP_OK) {
        ESP_LOGE(TAG, "GPIO config failed: %d", err);
        return err;
    }

    if ((err = gpio_install_isr_service(0)) != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "ISR service install failed: %d", err);
        return err;
    }

    if ((err = gpio_isr_handler_add(encoder->pin_a, encoder_isr_handler, encoder)) != ESP_OK ||
        (err = gpio_isr_handler_add(encoder->pin_b, encoder_isr_handler, encoder)) != ESP_OK ||
        (err = gpio_isr_handler_add(encoder->pin_btn, button_isr_handler, encoder)) != ESP_OK) {
        ESP_LOGE(TAG, "Handler add failed: %d", err);
        return err;
    }

    // Add hardware input filter
    gpio_set_filter_en(encoder->pin_a, true);
    gpio_set_filter_en(encoder->pin_b, true);
    gpio_set_filter_en(encoder->pin_btn, true);

    // Set filter timing (in nanoseconds)
    gpio_set_filter_period(encoder->pin_a, ENCODER_FILTER_NS);
    gpio_set_filter_period(encoder->pin_b, ENCODER_FILTER_NS);
    gpio_set_filter_period(encoder->pin_btn, ENCODER_FILTER_NS);

    // Initialize state
    encoder->counter = 0;
    encoder->btn_pressed = false;
    encoder->last_AB = (gpio_get_level(encoder->pin_a) << 1) |
                       gpio_get_level(encoder->pin_b);
    encoder->min_count = INT32_MIN;
    encoder->max_count = INT32_MAX;
    encoder->on_change = NULL;
    encoder->btn_pressed = false;

    ESP_LOGI(TAG, "Encoder initialized on pins A:%d B:%d BTN:%d",
             encoder->pin_a, encoder->pin_b, encoder->pin_btn);

    // Create event queue
    encoder->event_queue = xQueueCreate(10, sizeof(encoder_event_t));
    if (!encoder->event_queue) {
        ESP_LOGE(TAG, "Failed to create event queue");
        return ESP_ERR_NO_MEM;
    }

    return ESP_OK;
}

void encoder_set_bounds(encoder_config_t *encoder, int32_t min, int32_t max)
{
    if (encoder && min <= max) {
        encoder->min_count = min;
        encoder->max_count = max;
        // Clamp current value to new bounds
        if (encoder->counter < min)
            encoder->counter = min;
        if (encoder->counter > max)
            encoder->counter = max;
    }
}