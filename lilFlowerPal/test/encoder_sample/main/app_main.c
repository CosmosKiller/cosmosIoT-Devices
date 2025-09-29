#include "encoder_sample.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "main";

void app_main(void)
{
    encoder_config_t encoder = {
        .pin_a = PIN_ENCODER_A,
        .pin_b = PIN_ENCODER_B,
        .pin_btn = PIN_ENCODER_BTN};

    encoder_init(&encoder);
    ESP_LOGI(TAG, "Encoder initialized. Rotate or press the button.");

    while (1) {
        int32_t count = encoder_get_count(&encoder);
        bool btn = encoder_get_button(&encoder);

        ESP_LOGI(TAG, "Button pressed: %s", btn ? "YES" : "NO");
        ESP_LOGI(TAG, "Counter: %ld", count);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}