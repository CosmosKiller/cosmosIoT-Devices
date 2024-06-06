#include <string.h>

#include "cosmos_socket.h"
#include "socket_local_control.h"

// Output devices info
static cosmos_devices_t output_devs[] = {
    {
        .sn = "SKTMg-aaa0000",
        .pin = {23, 0, 0},
        .state = 0,
    },
    {
        .sn = "SKTMg-aaa0001",
        .pin = {22, 0, 0},
        .state = 0,
    },
    {
        .sn = "SKTMg-aaa0002",
        .pin = {21, 0, 0},
        .state = 0,
    },
    {
        .sn = "SKTMg-aaa0003",
        .pin = {19, 0, 0},
        .state = 0,
    },
};
static const int out_dev_qty = sizeof(output_devs) / sizeof(output_devs[0]);

// Defining button pins
static int button_pins[] = {34, 35, 32, 33};
static const int btn_qty = sizeof(button_pins) / sizeof(button_pins[0]);

/**
 * @brief Function to be excecuted when an ISR is
 * triggred by a button.
 *
 * @param arg Arguments which can be passed to
 * the ISR handler.
 */
static void socket_local_control_isr_handler(void *arg)
{
    // Get the device serial number from the argument
    char *dev_serial = (char *)arg;

    if (strncmp("SKT", dev_serial, 3) == 0)
        cosmos_socket_control(dev_serial, output_devs, out_dev_qty);
}

void socket_local_control_start(void)
{
    cosmos_devices_button_monitor(button_pins, btn_qty, socket_local_control_isr_handler, output_devs);
}