#ifndef MAIN_ENCODER_TASK_H_
#define MAIN_ENCODER_TASK_H_

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include <driver/gpio.h>
#include <driver/pulse_cnt.h>
#include <esp_err.h>

#define ENCODER_PIN_A   GPIO_NUM_4
#define ENCODER_PIN_B   GPIO_NUM_17
#define ENCODER_PIN_BTN GPIO_NUM_16

#define ENCODER_FILTER_NS   1000 /*!< 1000ns hardware filter */
#define ENCODER_DEBOUNCE_MS 5    /*!< 5ms software debounce */

typedef void (*encoder_count_callback_t)(int32_t count);
typedef void (*encoder_button_callback_t)(bool pressed);

/**
 * @brief Encoder event types
 *
 */
typedef enum {
    ENCODER_EVENT_COUNT,
    ENCODER_EVENT_BUTTON
} encoder_event_type_t;

/**
 * @brief Encoder event structure
 *
 */
typedef struct {
    encoder_event_type_t type;
    union {
        int32_t count;
        bool pressed;
    };
} encoder_event_t;

/**
 * @brief Rotary encoder configuration structure
 *
 */
typedef struct {
    gpio_num_t pin_a;                    /*!< CLK or A pin */
    gpio_num_t pin_b;                    /*!< DT or B pin */
    gpio_num_t pin_btn;                  /*!< SW or button pin */
    volatile int last_AB;                /*!< Last state of A and B pins */
    volatile int32_t counter;            /*!< Current counter value */
    volatile bool btn_pressed;           /*!< Button press flag */
    uint32_t last_change_time;           /*!< Debounce timing */
    int32_t min_count;                   /*!< Minimum counter value */
    int32_t max_count;                   /*!< Maximum counter value */
    encoder_count_callback_t on_change;  /*!< Counter change callback */
    encoder_button_callback_t on_button; /*!< Button event callback */
    QueueHandle_t event_queue;           /*!< Queue for encoder events */
} encoder_config_t;

/**
 * @brief Initialize the rotary encoder
 *
 * @param encoder
 * @return esp_err_t
 */
esp_err_t encoder_init(encoder_config_t *encoder);

/**
 * @brief Set min and max bounds for the encoder counter
 *
 * @param encoder
 * @param min
 * @param max
 */
void encoder_set_bounds(encoder_config_t *encoder, int32_t min, int32_t max);

#endif /* MAIN_ENCODER_TASK_H_ */