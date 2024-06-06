#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "cosmos_sensor.h"

// Handle for cosmos_sensor_begin
static bool s_sensor_begin_handle = false;

// Pointer to hold ADC calibration data
esp_adc_cal_characteristics_t *adc_chars;

/**
 * @brief Sensor setup
 *
 * @param pSensor Pointer to the strutct that contains all of the info
 * about the sensors used in the project
 */
static void cosmos_sensor_begin(cosmos_sensor_t *pSensor, size_t snr_qty)
{
    int snr_idx;

    /*
     * For ADC config. We'll be using channel 1,
     * as channel 2 is used by the WiFi driver
     */
    adc1_config_width(ADC_WIDTH_BIT_12);
    for (snr_idx = 0; snr_idx < snr_qty; snr_idx++) {
        adc1_config_channel_atten(INT_TO_ADC_CHANNEL(pSensor[snr_idx].pin_num[0]), ADC_ATTEN_DB_11);
    }

    // Perform ADC characterisation using the default V_REF calibration value
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);

    s_sensor_begin_handle = true;
}

/**
 * @brief As the ESP32 ADC can be sensitive to noise leading to large
 * discrepancies in ADC readings, multisampling may be used to
 * mitigate the effects of noise.
 *
 * @note Also, if possible, consider adding a 0.1uF capacitor to the
 * ADC input pad in use, when designing the PCB
 *
 * @param pSensor Pointer to the struct that contains the information of
 * the analog sensors
 * @return uint32_t Multisampled readings from adc1
 */
static uint32_t cosmos_sensor_adc1_multisampling(cosmos_sensor_t *pSensor)
{
    uint32_t adc_reading = 0;
    // Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        adc_reading += adc1_get_raw(INT_TO_ADC_CHANNEL(pSensor->pin_num[0]));
    }
    adc_reading /= NO_OF_SAMPLES;

    return adc_reading;
}

uint32_t cosmos_sensor_adc_read(cosmos_sensor_t *pSensor, size_t qty)
{
    // Check if sensor controller is configured
    if (s_sensor_begin_handle == false)
        cosmos_sensor_begin(pSensor, qty);

    uint32_t adc_reading = cosmos_sensor_adc1_multisampling(pSensor);
    // Convert ADC reading to voltage
    uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);

    return voltage;
}