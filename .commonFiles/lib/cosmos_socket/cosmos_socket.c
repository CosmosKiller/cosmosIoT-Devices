#include <string.h>

#include "driver/gpio.h"

#include "cosmos_socket.h"

// Handle for cosmos_socket_begin
static bool s_socket_begin_handle = false;

/**
 * @brief Socket setup function
 *
 * @param pSocket Pointer to the strutct that contains all of the info
 * about the socktes used in the project
 * @param dev_qty Quantity of sockets used in the project
 */
static void cosmos_socket_begin(cosmos_devices_t *pSocket, size_t dev_qty)
{
    int dev_idx;

    // Configure the GPIO for each socket
    for (dev_idx = 0; dev_idx < dev_qty; dev_idx++) {
        if (strncmp("SKT", pSocket[dev_idx].sn, 3) == 0) {
            gpio_pad_select_gpio(pSocket[dev_idx].pin[0]);
            gpio_set_direction(pSocket[dev_idx].pin[0], GPIO_MODE_OUTPUT);
            gpio_intr_disable(pSocket[dev_idx].pin[0]);

            /*
             * Set the gpio pin level to the innital state
             * defined in the devices struct of the main.c file
             */
            gpio_set_level(pSocket[dev_idx].pin[0], pSocket[dev_idx].state);
        }
    }

    s_socket_begin_handle = true;
}

void cosmos_socket_control(const char *sn_value, cosmos_devices_t *pSocket, size_t qty)
{
    // Check if socket controller is configured
    if (s_socket_begin_handle == false)
        cosmos_socket_begin(pSocket, qty);

    int skt_idx;

    // Loop through the different sockets
    for (skt_idx = 0; skt_idx < qty; skt_idx++) {
        /*
         * If the serial number matches
         * any of the stored in the
         * pSocket array, excution will
         * continue. Otherwise, we'll try
         * another with socket
         */
        if (strcmp(sn_value, pSocket[skt_idx].sn) == 0) {

            char *pDevState, new_msg[40];

            if (pSocket[skt_idx].state == 1) {
                gpio_set_level(pSocket[skt_idx].pin[0], 0);
                pSocket[skt_idx].state = 0;
                pDevState = "0|";
            } else {
                gpio_set_level(pSocket[skt_idx].pin[0], 1);
                pSocket[skt_idx].state = 1;
                pDevState = "1|";
            }

            memset(new_msg, '\0', sizeof(new_msg));
            strcpy(new_msg, pDevState);
            strcat(new_msg, sn_value);
            strcat(new_msg, "/rx_state");
        }
    }
}