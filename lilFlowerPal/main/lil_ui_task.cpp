/**
 * @file lil_ui_task.cpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2025-10-09
 *
 * @copyright Copyright (c) 2025
 *
 */
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <lil_ui_task.h>

static const char *TAG = "lil_ui_task";

static lv_obj_t *screens[LIL_UI_TASK_SCREEN_COUNT] = {0};
static int current_screen = 0;

// Encoder configuration
encoder_config_t encoder = {
    .pin_a = ENCODER_PIN_A,
    .pin_b = ENCODER_PIN_B,
    .pin_btn = ENCODER_PIN_BTN,
};

/**
 * @brief Task to handle screen switching based on rotary encoder input
 *
 * @param pArg Pointer to encoder configuration
 */
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

            ESP_LOGD(TAG, "Idx is %d", idx);

            if (idx < 0)
                current_screen = PCNT_HIGH_LIMIT + idx; // Wrap around to last screen
            else
                current_screen = idx;

            pEncoder->counter = idx;
            ESP_LOGD(TAG, "Switching to screen %d", current_screen + 1);
            lv_scr_load(screens[current_screen]);
        }

        // Add delay to prevent tight loop
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void lil_ui_task_start(void)
{
    // Initialize encoder
    encoder_init(&encoder);

    // Create event handling task
    xTaskCreate(lil_ui_task_switch_screen, "lil_ui_task_switch_screen", LIL_UI_TASK_STACK_SIZE, &encoder, LIL_UI_TASK_PRIORITY, NULL);
}