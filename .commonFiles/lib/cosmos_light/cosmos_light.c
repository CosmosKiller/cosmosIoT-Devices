#include <stdbool.h>
#include <string.h>

#include "cosmos_light.h"

// Handle for cosmos_ligth_begin
static bool s_light_begin_handle = false;

/**
 * @brief Initializes the RGB LED settings per channel, including
 * the GPIOS for each color, mode and timer configuration.
 *
 * @param pLight Pointer to the strutct that contains all of the info
 * @param lsc_qty Quantity of LEDs used in the project
 */
static void cosmos_light_begin(cosmos_light_info_t *pLight, size_t lsc_qty)
{
    char *pRet;
    int led_idx, ch_idx;

    // Initialize timer zero config struct
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 100,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
    };
    ledc_timer_config(&ledc_timer);

    // Zero-initialize ledc channel config struct
    ledc_channel_config_t ledc_channel[lsc_qty][RGB_LED_CHANNEL_NUM];
    memset(ledc_channel, 0, lsc_qty * RGB_LED_CHANNEL_NUM * sizeof(ledc_channel_config_t));

    // Configure channels
    for (led_idx = 0; led_idx < lsc_qty; led_idx++) {
        ledc_channel[led_idx][0].gpio_num = pLight[led_idx].pDevice->pin[0];
        ledc_channel[led_idx][0].channel = pLight[led_idx].channel[0];
        ledc_channel[led_idx][0].duty = 0;
        ledc_channel[led_idx][0].hpoint = 0;
        ledc_channel[led_idx][0].intr_type = LEDC_INTR_DISABLE;
        ledc_channel[led_idx][0].speed_mode = pLight[led_idx].mode;
        ledc_channel[led_idx][0].timer_sel = pLight[led_idx].timer_index;

        // Set LED Controller with previously prepared configuration
        ledc_channel_config(&ledc_channel[led_idx][0]);

        /*
         * Check if the LED subtype (Look for types
         * and subtypes definition in the following
         * link -> ) is 'color' (c). If so we'll
         * configure the rest of the LED channels.
         */
        pRet = strpbrk(pLight[led_idx].pDevice->sn, "c");
        if (pRet) {
            for (ch_idx = 1; ch_idx < 3; ch_idx++) {
                ledc_channel[led_idx][ch_idx].gpio_num = pLight[led_idx].pDevice->pin[ch_idx];
                ledc_channel[led_idx][ch_idx].channel = pLight[led_idx].channel[ch_idx];
                ledc_channel[led_idx][ch_idx].duty = 0;
                ledc_channel[led_idx][ch_idx].hpoint = 0;
                ledc_channel[led_idx][ch_idx].intr_type = LEDC_INTR_DISABLE;
                ledc_channel[led_idx][ch_idx].speed_mode = pLight[led_idx].mode;
                ledc_channel[led_idx][ch_idx].timer_sel = pLight[led_idx].timer_index;

                ledc_channel_config(&ledc_channel[led_idx][ch_idx]);
            }
        }
    }

    s_light_begin_handle = true;
}

void cosmos_light_control(const char sn_value[14], char rgb_values[17], cosmos_light_info_t *pLight, size_t qty)
{
    // Check if LED controller is configured
    if (s_light_begin_handle == false)
        cosmos_light_begin(pLight, qty);

    int led_idx;

    // Loop through the different LEDs
    for (led_idx = 0; led_idx < qty; led_idx++) {
        /*
         * If the serial number matches
         * any of the stored in the
         * pLight array, excution will
         * continue. Otherwise, we'll try
         * another with LED
         */
        if (strcmp(sn_value, pLight[led_idx].pDevice->sn) == 0) {

            int rgb_val, rgb_idx, rgb_arr[4];
            char *pRet, *pDevState, new_msg[40];

            rgb_val = 0;
            /*
             * This loop will break down the rgb values
             * and store them in the 'rgb_arr' array
             */
            for (rgb_idx = 0; rgb_idx < 4; rgb_idx++) {
                char aux_value[4];
                /*
                 * Each value is represented as a
                 * 3-digit integer, we copy the 3
                 * chars before the forward slash
                 * to the 'aux_value' string
                 */
                strncpy(aux_value, &rgb_values[rgb_val], 3);
                rgb_val += 4;

                // Convert the str 'aux_value' to an int and store it
                rgb_arr[rgb_idx] = atoi(aux_value);
            }

            int r_value = 0, g_value = 0, b_value = 0;

            // Adjusts the duty of the channels based on the brightness value
            r_value = COSMOS_MAP(rgb_arr[1], 0, 100, 0, rgb_arr[0]);
            g_value = COSMOS_MAP(rgb_arr[2], 0, 100, 0, rgb_arr[0]);
            b_value = COSMOS_MAP(rgb_arr[3], 0, 100, 0, rgb_arr[0]);

            // Set and update duty for the first led channel
            ledc_set_duty(pLight[led_idx].mode, pLight[led_idx].channel[0], r_value);
            ledc_update_duty(pLight[led_idx].mode, pLight[led_idx].channel[0]);

            /*
             * Check if the LED subtype (Look for types
             * and subtypes definition in the following
             * link -> ) is 'color' (c). If so, we'll
             * set and update the duty the rest of the
             * LED channels.
             */
            pRet = strpbrk(pLight[led_idx].pDevice->sn, "c");
            if (pRet) {
                ledc_set_duty(pLight[led_idx].mode, pLight[led_idx].channel[1], g_value);
                ledc_update_duty(pLight[led_idx].mode, pLight[led_idx].channel[1]);

                ledc_set_duty(pLight[led_idx].mode, pLight[led_idx].channel[2], b_value);
                ledc_update_duty(pLight[led_idx].mode, pLight[led_idx].channel[2]);
            }

            if (strcmp(rgb_values, "000/000/000/000/") == 0) {
                pLight[led_idx].pDevice->state = 0;
                pDevState = "0|";
            } else {
                pLight[led_idx].pDevice->state = 1;
                pLight[led_idx].rgb_values = rgb_values;
                pDevState = "1|";
            }

            memset(new_msg, '\0', sizeof(new_msg));
            strcpy(new_msg, pDevState);
            strcat(new_msg, sn_value);
            strcat(new_msg, "/rx_state");
        }
    }
}