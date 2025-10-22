#ifndef MAIN_COSMOS_LIGHT_H_
#define MAIN_COSMOS_LIGHT_H_

#include "driver/ledc.h"

#include "cosmos_devices.h"

#define RGB_LED_CHANNEL_NUM 3

/**
 * @brief RGB LED configuration struct
 *
 */
typedef struct {
    cosmos_devices_t *pDevice; /*!< Struct that contains basic info */
    char *rgb_values;          /*!< String that contains Brigthness and RGB values */
    ledc_channel_t channel[3]; /*!< LEDC Channel */
    ledc_mode_t mode;          /*!< LEDC speed speed_mode */
    ledc_timer_t timer_index;  /*!< Timer source of the channel */
} cosmos_light_info_t;

/**
 * @brief This function is used to control color and brightness of
 * any RGB ligth source

 * @param sn_value Serial number used to compare and
 * determine with which ligth source we are trying to interact
 * @param rgb_values Array that contains Brigthness and RGB values.
 * Each of them must be in the following order -> "Bright/Rvalue/Gvalue/Bvalue/" E.g. 050/255/112/017/
 * @param pLight Pointer to the strutct that contains all of the info
 * about the ligth sources used in the project
 * @param qty Quantity of light sources used in the project
 */
void cosmos_light_control(const char sn_value[14], char rgb_values[17], cosmos_light_info_t *pLight, size_t qty);

#endif /* MAIN_COSMOS_LIGHT_H_ */