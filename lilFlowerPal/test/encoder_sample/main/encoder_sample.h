#pragma once

#include <driver/gpio.h>
#include <esp_err.h>

#define PIN_ENCODER_A   GPIO_NUM_4
#define PIN_ENCODER_B   GPIO_NUM_17
#define PIN_ENCODER_BTN GPIO_NUM_16

typedef struct {
    gpio_num_t pin_a;          /*!< CLK or A pin */
    gpio_num_t pin_b;          /*!< DT or B pin */
    gpio_num_t pin_btn;        /*!< SW or button pin */
    volatile int last_AB;      /*!< Last state of A and B pins */
    volatile int32_t counter;  /*!< Current counter value */
    volatile bool btn_pressed; /*!< Button press flag */
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
 * @brief Get the current counter value
 *
 */
int32_t encoder_get_count(encoder_config_t *encoder);

/**
 * @brief Check if the button was pressed since last call
 *
 */
bool encoder_get_button(encoder_config_t *encoder);