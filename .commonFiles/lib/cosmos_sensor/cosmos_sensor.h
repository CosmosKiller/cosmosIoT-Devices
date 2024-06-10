#ifndef MAIN_COSMOS_SENSOR_H_
#define MAIN_COSMOS_SENSOR_H_

#define NO_OF_SAMPLES 64   /*!< Standar sample rate for ADC multisampling */
#define DEFAULT_VREF  1100 /*!< */

#define INT_TO_ADC_CHANNEL(snr_pin) (snr_pin > 35 ? snr_pin - 36 : snr_pin - 28) /*!< Because the GPIO pin is expresed as an int, we need to convert it to an adc1_channel_t */

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
    const int *pin_num;              /*!< Pin number in which the sensor is connected */
    cosmos_sensor_reading_u reading; /*!< Min threshold Level */
    cosmos_sensor_type_e type;       /*!< Sensor type */
} cosmos_sensor_t;

/**
 * @brief This function configures and characterize the ADC at
 * 11db attenuation and a bandwidth of 12bits.
 *
 * It also takes multisampled reading from ADC1.
 *
 * @param pSensor Pointer to the struct which contains the sensor's
 * information
 * @param qty Quantity of sensors used im the project
 */
uint32_t cosmos_sensor_adc_read(cosmos_sensor_t *pSensor, size_t qty);

#endif /* MAIN_COSMOS_SENSOR_H_ */