#ifndef MAIN_COSMOS_SENSOR_H_
#define MAIN_COSMOS_SENSOR_H_

#include "esp_adc/adc_cali.h"

#define NO_OF_SAMPLES 64   /*!< Standard sample rate for ADC multisampling */
#define DEFAULT_VREF  1100 /*!< By design, the ADC reference voltage for ESP32 is 1100 mV */

/**
 * @brief Types of sensor
 *
 */
typedef enum {
    SNR_TYPE_TH = 0, /*!< Temperature sensor */
    SNR_TYPE_WL,     /*!< Water level sensor */
    SNR_TYPE_SM,     /*!< Soil moisture sensor */
    SNR_TYPE_PO,     /*!< Air Polution sensor */
    SNR_TYPE_FM,     /*!< Flowmeter sensor */
} cosmos_sensor_type_e;

/**
 * @brief
 *
 */
typedef union {
    int i;   /*!< For percentual values */
    float f; /*!< For non-percentual values */
} cosmos_sensor_reading_u;

/**
 * @brief Use this struct
 * to store the parameters
 * of the sensors
 */
typedef struct {
    cosmos_sensor_type_e type;    /*!< Sensor type */
    const int pin_num;            /*!< Pin number in which the sensor is connected */
    adc_channel_t snr_chn;        /*!< ADC Channel associated to the sensor pin. Refere to [this link](https://lastminuteengineers.com/esp32-wroom-32-pinout-reference/#esp32wroom32-adc-pins) for a correct designation of pins and ADC channels */
    adc_cali_handle_t snr_handle; /*!< Sensor calibration handle */
    bool cali_flag;               /*!< Calibration flag. Set to false when first declaring the sensor */
    int reading;                  /*!< Sensor readings. Value interpretation depends of the sensor type */
} cosmos_sensor_t;

/**
 * @brief Configures and characterize the ADC at
 * 12db attenuation and a bandwidth of 12bits.
 *
 * It also takes multisampled reading from ADC1.
 *
 * @param pSensor Pointer to the struct which contains the sensor's
 * information
 * @param snr_qty Quantity of sensors used in the project
 */
void cosmos_sensor_adc_read_raw(cosmos_sensor_t *pSensor, int snr_qty);

#endif /* MAIN_COSMOS_SENSOR_H_ */