#include "toggle.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED_GPIO    GPIO_NUM_4
#define BUTTON_GPIO GPIO_NUM_16

static void toggle_init()
{
    // LED config
    gpio_config_t led_conf = {
        .pin_bit_mask = (1ULL << LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&led_conf);

    // Button config
    gpio_config_t btn_conf = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&btn_conf);
}

static void toggle_task(void *pvParameters)
{
    bool led_on = false;
    int last_state = 1;

    while (1) {
        int current_state = gpio_get_level(BUTTON_GPIO);

        if (last_state == 1 && current_state == 0) {
            led_on = !led_on;
            gpio_set_level(LED_GPIO, led_on);
        }

        last_state = current_state;
        vTaskDelay(pdMS_TO_TICKS(50)); // debounce
    }
}

void toggle_start()
{
    toggle_init();

    // Create the task
    xTaskCreatePinnedToCore(toggle_task, "toggle_task", 2048, NULL, 5, NULL, 1);
}
