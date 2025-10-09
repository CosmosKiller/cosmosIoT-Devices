#include <encoder_task.h>
#include <lvgl_task.h>

static const char *TAG = "lil_ui_task";

lv_obj_t *screens[4];
static int32_t current_screen = 0;

static void lil_ui_task_switch_screen(int32_t idx)
{
    if (idx < 0)
        idx = 3;
    if (idx > 3)
        idx = 0;

    current_screen = idx;
    lv_scr_load(screens[current_screen]);
    ESP_LOGI(TAG, "Switched to screen %ld", current_screen);
}

static void lil_ui_task_event_handling(void *arg)
{
    encoder_config_t *encoder = (encoder_config_t *)arg;
    encoder_event_t evt;

    while (1) {
        if (xQueueReceive(encoder->event_queue, &evt, portMAX_DELAY)) {
            switch (evt.type) {
            case ENCODER_EVENT_COUNT:
                if (encoder->on_change) {
                    encoder->on_change(evt.count);
                }
                lil_ui_task_switch_screen(evt.count);
                break;

            case ENCODER_EVENT_BUTTON:
                if (encoder->on_button) {
                    encoder->on_button(evt.pressed);
                }
                ESP_LOGI(TAG, "Button %s", evt.pressed ? "pressed" : "released");
                break;
            }
        }
    }
}

void lil_ui_task_start(void)
{
    encoder_config_t encoder = {
        .pin_a = ENCODER_PIN_A,
        .pin_b = ENCODER_PIN_B,
        .pin_btn = ENCODER_PIN_BTN};

    encoder_init(&encoder);
    encoder_set_bounds(&encoder, 0, 3);
    encoder_set_callbacks(&encoder, on_count_changed, on_button_event);

    // Create event handling task
    xTaskCreate(encoder_event_task, "encoder_events", 2048,
                &encoder, 5, NULL);

    // Main loop
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}