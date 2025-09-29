#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "smart_desk_tasks_common.h"

#define ENCODER_PIN_A GPIO_NUM_17
#define ENCODER_PIN_B GPIO_NUM_4

/**
 * @brief
 *
 * @param pvParameter
 */
static void encoder_task(void *pvParameter)
{
    int position = 0;
    int last_AB = 0;
    int new_AB = 0;

    while (1) {
        // Read the current state of the encoder pins
        int A = gpio_get_level(ENCODER_PIN_A);
        int B = gpio_get_level(ENCODER_PIN_B);

        // Combine the states of the two pins into one value
        new_AB = (A << 1) | B;

        // If the new state is different from the last state
        if (new_AB != last_AB) {
            // Check the direction of rotation
            if ((last_AB == 0b00 && new_AB == 0b01) ||
                (last_AB == 0b01 && new_AB == 0b11) ||
                (last_AB == 0b11 && new_AB == 0b10) ||
                (last_AB == 0b10 && new_AB == 0b00)) {
                position++;
            } else {
                position--;
            }
            printf("Position: %d\n", position);
        }

        // Update last state
        last_AB = new_AB;

        // Delay to avoid high CPU usage
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void encoder_task_start()
{
    // Configure encoder pins as inputs
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << ENCODER_PIN_A) | (1ULL << ENCODER_PIN_B);
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    gpio_config(&io_conf);

    // Create a task to read the encoder position
    xTaskCreatePinnedToCore(&encoder_task, "encoder_task", ENCODER_TASK_STACK_SIZE, NULL, ENCODER_TASK_PRIORITY, NULL, ENCODER_TASK_CORE_ID);
}