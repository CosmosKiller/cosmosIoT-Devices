#include <string.h>

#include "driver/gpio.h"

#include "cosmos_power.h"

// Handle for cosmos_power_begin
static bool s_power_begin_handle = false;

/**
 * @brief Power device setup function
 *
 * @param pPower Pointer to the strutct that contains all of the info
 * about the socktes used in the project
 * @param dev_qty Quantity of power devices used in the project
 */
static void cosmos_power_begin(cosmos_devices_t *pPower, size_t dev_qty)
{
    int dev_idx;

    // Configure the GPIO for each power device
    for (dev_idx = 0; dev_idx < dev_qty; dev_idx++) {
        if (strncmp("PWR", pPower[dev_idx].sn, 3) == 0) {
            gpio_pad_select_gpio(pPower[dev_idx].pin[0]);
            gpio_set_direction(pPower[dev_idx].pin[0], GPIO_MODE_OUTPUT);
            gpio_intr_disable(pPower[dev_idx].pin[0]);

            /*
             * Set the gpio pin level to the innital state
             * defined in the devices struct of the main.c file
             */
            gpio_set_level(pPower[dev_idx].pin[0], pPower[dev_idx].state);
        }
    }

    s_power_begin_handle = true;
}

void cosmos_power_control(const char *sn_value, cosmos_devices_t *pPower, size_t qty)
{
    // Check if power controller is configured
    if (s_power_begin_handle == false)
        cosmos_power_begin(pPower, qty);

    int pwr_idx;

    // Loop through the different power devices
    for (pwr_idx = 0; pwr_idx < qty; pwr_idx++) {
        /*
         * If the serial number matches
         * any of the stored in the
         * pPower array, excution will
         * continue. Otherwise, we'll try
         * another with power
         */
        if (strcmp(sn_value, pPower[pwr_idx].sn) == 0) {

            char *pDevState, new_msg[40];

            if (pPower[pwr_idx].state == 1) {
                gpio_set_level(pPower[pwr_idx].pin[0], 0);
                pPower[pwr_idx].state = 0;
                pDevState = "0|";
            } else {
                gpio_set_level(pPower[pwr_idx].pin[0], 1);
                pPower[pwr_idx].state = 1;
                pDevState = "1|";
            }

            memset(new_msg, '\0', sizeof(new_msg));
            strcpy(new_msg, pDevState);
            strcat(new_msg, sn_value);
            strcat(new_msg, "/rx_state");
        }
    }
}