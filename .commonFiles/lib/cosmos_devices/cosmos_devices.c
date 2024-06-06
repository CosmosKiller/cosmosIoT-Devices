#include <string.h>

#include "cosmos_devices.h"

// Handle for cosmos_devices_btn_begin
static bool s_button_begin_handle = false;

/**
 * @brief Button ISR setup function
 *
 * @param pButton Pins in which the buttons are be connected
 * @param btn_qty Quantity of buttons used in the ISR
 * @param button_isr_handler ISR handler function
 * @param pDevices Pointer to strutct that contains the
 * devices to be handled
 */
static void cosmos_devices_button_begin(int *pButton, size_t btn_qty, gpio_isr_t button_isr_handler, cosmos_devices_t *pDevices)
{
    int btn_idx;

    // Configure the GPIO for each button
    for (btn_idx = 0; btn_idx < btn_qty; btn_idx++) {
        gpio_pad_select_gpio(pButton[btn_idx]);
        gpio_set_direction(pButton[btn_idx], GPIO_MODE_INPUT);
        gpio_set_pull_mode(pButton[btn_idx], GPIO_PULLUP_ONLY);
        gpio_intr_enable(pButton[btn_idx]);
        gpio_set_intr_type(pButton[btn_idx], GPIO_INTR_POSEDGE);

        // Install the ISR service
        gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);

        // Register the ISR
        gpio_isr_handler_add(pButton[btn_idx], button_isr_handler, (void *)pDevices[btn_idx].sn);
    }

    s_button_begin_handle = true;
}

void cosmos_devices_button_monitor(int *pButton, size_t qty, gpio_isr_t button_isr_handler, cosmos_devices_t *pDevices)
{
    // Check if button monitor is configured
    if (s_button_begin_handle == false)
        cosmos_devices_button_begin(pButton, qty, button_isr_handler, pDevices);
}