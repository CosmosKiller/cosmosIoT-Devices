#ifndef MAIN_ENCODER_TASK_H_
#define MAIN_ENCODER_TASK_H_

#include "freertos/queue.h"
#include <driver/gpio.h>
#include <esp_err.h>

#define PIN_ENCODER_A   GPIO_NUM_4
#define PIN_ENCODER_B   GPIO_NUM_17
#define PIN_ENCODER_BTN GPIO_NUM_16

#define QUEUE_HANDLING 1 /*!< Use queue to handle events in a separate task */

// Acceleration config
#define ENCODER_ACCEL_THRESHOLD_MS 75 // Time threshold for acceleration
#define ENCODER_ACCEL_FACTOR_MAX   4  // Maximum acceleration multiplier

typedef void (*encoder_count_callback_t)(int32_t count);
typedef void (*encoder_button_callback_t)(bool pressed);

typedef enum {
    ENCODER_EVENT_COUNT,
    ENCODER_EVENT_BUTTON
} encoder_event_type_t;

typedef struct {
    encoder_event_type_t type;
    union {
        int32_t count;
        bool pressed;
    };
} encoder_event_t;

typedef struct {
    gpio_num_t pin_a;                    /*!< CLK or A pin */
    gpio_num_t pin_b;                    /*!< DT or B pin */
    gpio_num_t pin_btn;                  /*!< SW or button pin */
    volatile int last_AB;                /*!< Last state of A and B pins */
    volatile int32_t counter;            /*!< Current counter value */
    volatile bool btn_pressed;           /*!< Button press flag */
    int32_t min_count;                   /*!< Minimum counter value */
    int32_t max_count;                   /*!< Maximum counter value */
    uint32_t last_change_time;           /*!< Last encoder change timestamp */
    uint8_t acceleration;                /*!< Current acceleration factor */
    encoder_count_callback_t on_change;  /*!< Counter change callback */
    encoder_button_callback_t on_button; /*!< Button event callback */
    QueueHandle_t event_queue;           /*!< Queue for encoder events */
} encoder_config_t;

/**
 * @brief Limits the counter value between min and max values
 *        Prevents counting beyond set limits
 *        Useful for menu selections, volume control, etc.
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

/**
 * @brief
 *
 * @param encoder
 * @param count_cb
 * @param button_cb
 */
void encoder_set_callbacks(encoder_config_t *encoder,
                           encoder_count_callback_t count_cb,
                           encoder_button_callback_t button_cb);

#endif /* MAIN_ENCODER_TASK_H_ */