#ifndef MAIN_COSMOS_DEVICES_H_
#define MAIN_COSMOS_DEVICES_H_

#include <driver/gpio.h>

#define RX_CONTROL "/rx_control"
#define RX_STATE   "/rx_state"

#define COSMOS_MAP(x, in_min, in_max, out_min, out_max) ((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min) /*!< Arduino style map function */

#define QTY(devices) (sizeof(devices) / sizeof(*(devices))) /*!< Quantity of devices */

/**
 * @brief Standard structure for devices management
 * DON'T MODIFY THIS STRUCT!
 */
typedef struct {
    const char *sn;   /*!< Serial Number of the devices */
    const int pin[3]; /*!< Pins in which the device will be connected | For LED -> {rPin, gPin, bPin} | For other -> {devPin, 0, 0} */
    int state;        /*<! Initial state of the device (0: low ; 1: high) */
} cosmos_devices_t;

/**
 * @brief Types of devices avilable
 *
 */
typedef enum {
    DEVICE_TYPE_LSC = 0, /*!< Light Source */
    DEVICE_TYPE_PWR,     /*!< Power control device */
    DEVICE_TYPE_SNR,     /*!< Sensor */
    DEVICE_TYPE_CAM,     /*!< Camera */
    DEVICE_TYPE_MOT,     /*!< DC Motor */
} cosmos_devices_type_e;

/**
 * @brief Call this function whenever you need to
 * use buttons to locally control any device.
 *
 * @param pButton Pins in which the buttons are be connected
 * @param qty Quantity of buttons used in the ISR
 * @param button_isr_handler ISR handler function
 * @param pDevices Pointer to strutct that contains the
 * devices to be handled
 */
void cosmos_devices_button_monitor(int *pButton, size_t qty, gpio_isr_t button_isr_handler, cosmos_devices_t *pDevices);

#endif /* MAIN_COSMOS_DEVICES_H_ */