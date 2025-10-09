#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <lvgl.h>

#include <encoder_task.h>
#include <lvgl_task.h>

static const char *TAG = "lil_ui_task";

lv_obj_t *screens[4];
static int current_screen = 0;

// Encoder configuration
encoder_config_t encoder = {
    .pin_a = ENCODER_PIN_A,
    .pin_b = ENCODER_PIN_B,
    .pin_btn = ENCODER_PIN_BTN,
    .pcnt_unit = NULL,
};

static void lil_ui_task_switch_screen(void *pArg)
{
    encoder_config_t *pEncoder = (encoder_config_t *)pArg;
    int idx = 0;

    // Verify encoder configuration
    if (!pEncoder || !pEncoder->pcnt_unit) {
        ESP_LOGE(TAG, "Invalid encoder configuration");
        vTaskDelete(NULL);
        return;
    }

    while (1) {
        // Get the current count from the PCNT unit
        pcnt_unit_get_count(pEncoder->pcnt_unit, &idx);

        if (idx != pEncoder->counter) {

            ESP_LOGI(TAG, "Idx is %d", idx);

            if (idx < 0)
                current_screen = PCNT_HIGH_LIMIT + idx; // Wrap around to last screen
            else
                current_screen = idx;

            pEncoder->counter = idx;
            ESP_LOGI(TAG, "Switching to screen %d", current_screen + 1);
            lv_scr_load(screens[current_screen]);
        }

        // Add delay to prevent tight loop
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void app_main(void)
{
    encoder_init(&encoder);

    lvgl_task_start();

    // Create event handling task
    xTaskCreate(lil_ui_task_switch_screen, "lil_ui_task_switch_screen", 2048, &encoder, 5, NULL);

    // Main loop
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}