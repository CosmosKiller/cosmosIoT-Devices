#include "encoder_sample.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "encoder_task";

// Button ISR handler with debounce
static void IRAM_ATTR button_isr_handler(void *arg)
{
    static uint32_t last_button_time = 0;
    uint32_t current_time = xTaskGetTickCountFromISR();

    // Simple debounce - ignore presses within 50ms
    if ((current_time - last_button_time) >= pdMS_TO_TICKS(50)) {
        encoder_config_t *encoder = (encoder_config_t *)arg;
        encoder->btn_pressed = true;
        last_button_time = current_time;
    }
}

// Encoder ISR handler with improved state tracking
static void IRAM_ATTR encoder_isr_handler(void *arg)
{
    encoder_config_t *encoder = (encoder_config_t *)arg;

    // Read current state
    int A = gpio_get_level(encoder->pin_a);
    int B = gpio_get_level(encoder->pin_b);

    // Get new state
    int new_AB = (A << 1) | B;
    int old_AB = encoder->last_AB; // Use per-encoder state

    // Valid transition check and direction determination
    if (new_AB != old_AB) {
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
        // Store new state
        encoder->last_AB = new_AB;
    }
}

esp_err_t encoder_init(encoder_config_t *encoder)
{
    if (!encoder) {
        return ESP_ERR_INVALID_ARG;
    }

    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_ANYEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << encoder->pin_a) |
                        (1ULL << encoder->pin_b) |
                        (1ULL << encoder->pin_btn),
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE};

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

    // Initialize state
    encoder->counter = 0;
    encoder->btn_pressed = false;
    encoder->last_AB = (gpio_get_level(encoder->pin_a) << 1) |
                       gpio_get_level(encoder->pin_b);

    ESP_LOGI(TAG, "Encoder initialized on pins A:%d B:%d BTN:%d",
             encoder->pin_a, encoder->pin_b, encoder->pin_btn);
    return ESP_OK;
}

int32_t encoder_get_count(encoder_config_t *encoder)
{
    return encoder->counter;
}

bool encoder_get_button(encoder_config_t *encoder)
{
    bool pressed = encoder->btn_pressed;
    encoder->btn_pressed = false; // Clear the flag
    return pressed;
}