#ifndef MAIN_COSMOS_SENSOR_H_
#define MAIN_COSMOS_SENSOR_H_

#include "esp_adc/adc_cali.h"

#define NO_OF_SAMPLES 16   /*!< Standard sample rate for ADC multisampling */
#define FILTER_SIZE   10   /*!< moving average window size */
#define DEFAULT_VREF  1100 /*!< By design, the ADC reference voltage for ESP32 is 1100 mV */

#define COSMOS_MAP(x, in_min, in_max, out_min, out_max) ((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min) /*!< Arduino style map function */

/**
 * @brief
 *
 */
typedef struct {
    int buffer[FILTER_SIZE];
    int index;
    int count;
    long sum;
} cosmos_sensor_moving_avg_t;

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
 * @brief Use this struct
 * to store the parameters
 * of the sensors
 */
typedef struct {
    const int pin_num;                   /*!< Pin number in which the sensor is connected */
    adc_channel_t snr_chn;               /*!< ADC Channel associated to the sensor pin. Refere to [this link](https://lastminuteengineers.com/esp32-wroom-32-pinout-reference/#esp32wroom32-adc-pins) for a correct designation of pins and ADC channels */
    adc_cali_handle_t snr_handle = NULL; /*!< Sensor calibration handle */
    bool cali_flag = false;              /*!< Calibration flag. Set to false when first declaring the sensor */
    int reading = 0;                     /*!< Sensor readings. Value interpretation depends of the sensor type */
    cosmos_sensor_type_e snr_type;       /*!< Sensor type */
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
void cosmos_sensor_adc_read_voltage(cosmos_sensor_t *pSensor, int snr_qty);

#endif /* MAIN_COSMOS_SENSOR_H_ */