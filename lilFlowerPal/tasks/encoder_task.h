#ifndef MAIN_ENCODER_TASK_H_
#define MAIN_ENCODER_TASK_H_

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include <driver/gpio.h>
#include <driver/pulse_cnt.h>
#include <esp_err.h>

#include <main_tasks_common.h>

#define PCNT_HIGH_LIMIT LIL_UI_TASK_SCREEN_COUNT
#define PCNT_LOW_LIMIT  -LIL_UI_TASK_SCREEN_COUNT

#define ENCODER_PIN_A   GPIO_NUM_4
#define ENCODER_PIN_B   GPIO_NUM_17
#define ENCODER_PIN_BTN GPIO_NUM_16

#define ENCODER_FILTER_NS 1000 /*!< 1000ns hardware filter */

/**
 * @brief Rotary encoder configuration structure
 *
 */
typedef struct {
    gpio_num_t pin_a;
    gpio_num_t pin_b;
    gpio_num_t pin_btn;
    pcnt_unit_handle_t pcnt_unit = NULL;
    QueueHandle_t event_queue;
    int32_t counter;
} encoder_config_t;

/**
 * @brief Initialize the rotary encoder
 *
 * @param encoder
 * @return esp_err_t
 */
esp_err_t encoder_init(encoder_config_t *encoder);

#endif /* MAIN_ENCODER_TASK_H_ */