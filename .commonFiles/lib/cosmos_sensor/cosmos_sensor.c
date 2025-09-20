#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

#include "cosmos_sensor.h"

const static char *TAG = "COSMOS_SENSOR";

// Handle for cosmos_sensor_begin
static bool s_sensor_begin_handle = false;

// Handles for ADC channels 1 and 2
static adc_oneshot_unit_handle_t adc1_handle;
static adc_oneshot_unit_handle_t adc2_handle;

/**
 * @brief Configures and characterize the ADC at
 * 12db attenuation and a bandwidth of 12bits.
 *
 * @param pSensor Pointer to the strutct that contains all of the info
 * about the sensors used in the project
 * @param snr_qty Quantity of sensors used in the project
 */
static void cosmos_sensor_begin(cosmos_sensor_t *pSensor, int snr_qty)
{
    static bool adc_unit1_ready = false;
    static bool adc_unit2_ready = false;
    // Oneshot default params
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    // Cycle through all sensors
    for (int snr_idx = 0; snr_idx < snr_qty; snr_idx++) {

        // Check if sensor is connected to ADC1
        if (pSensor[snr_idx].pin_num >= 32) {

            if (adc_unit1_ready == false) {
                // ADC1 Init
                adc_oneshot_unit_init_cfg_t init_config1 = {
                    .unit_id = ADC_UNIT_1,
                };
                ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));
                adc_unit1_ready = true;
            }

            // ADC1 Config
            ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, pSensor->snr_chn, &config));
        } else {

            if (adc_unit2_ready == false) {
                // ADC2 Init
                adc_oneshot_unit_init_cfg_t init_config2 = {
                    .unit_id = ADC_UNIT_2,
                    .ulp_mode = ADC_ULP_MODE_DISABLE,
                };
                ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config2, &adc2_handle));
                adc_unit2_ready = true;
            }

            // ADC2 Config
            ESP_ERROR_CHECK(adc_oneshot_config_channel(adc2_handle, pSensor->snr_chn, &config));
        }
    }
    ESP_LOGI(TAG, "Init Success");
    s_sensor_begin_handle = true;
}

/**
 * @brief As the ESP32 ADC can be sensitive to noise leading to large
 * discrepancies in ADC readings, multisampling may be used to
 * mitigate the effects of noise.
 *
 * @note Also, if possible, consider adding a 0.1uF capacitor to the
 * ADC input pad in use, when designing the PCB
 * @param pSensor Pointer to the struct that contains the information of
 * the analog sensors
 * @return int Multisampled readings from adc1
 */
static int cosmos_sensor_adc1_multisampling(cosmos_sensor_t *pSensor)
{
    int adc_reading = 0;
    // Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        adc_oneshot_read(adc1_handle, pSensor->snr_chn, &pSensor->reading);
        adc_reading += pSensor->reading;
    }
    adc_reading /= NO_OF_SAMPLES;

    return adc_reading;
}

/**
 * @brief Checks if any calibration scheme is supported by the ADC.
 *
 * @param unit ADC unit associated to the sensor pin.
 * @param channel ADC channel associated to the sensor pin.
 * @param out_handle Calibration hadle
 * @param calibrated Calibration flag
 */
static void cosmos_sensor_adc_cali_init(adc_unit_t unit, adc_channel_t channel, adc_cali_handle_t *out_handle, bool *calibrated)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret_msg = ESP_FAIL;
    *calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (*calibrated == false) {
        ESP_LOGI(TAG, "Calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_12,
        };
        ret_msg = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret_msg == ESP_OK) {
            *calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (*calibrated == false) {
        ESP_LOGI(TAG, "Calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_12,
            .default_vref = DEFAULT_VREF,
        };
        ret_msg = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret_msg == ESP_OK) {
            *calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret_msg == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret_msg == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }
}

void cosmos_sensor_adc_read_raw(cosmos_sensor_t *pSensor, int snr_qty)
{
    // Check if sensor controller is configured
    if (s_sensor_begin_handle == false)
        cosmos_sensor_begin(pSensor, snr_qty);

    // Cycle through all sensors
    for (int snr_idx = 0; snr_idx < snr_qty; snr_idx++) {

        // Check if sensor is connected to ADC1
        if (pSensor[snr_idx].pin_num >= 32) {

            // Check sensor calibration
            if (pSensor[snr_idx].cali_flag == false) {

                // Sensor calibration
                cosmos_sensor_adc_cali_init(ADC_UNIT_1, pSensor[snr_idx].snr_chn, &pSensor[snr_idx].snr_handle, &pSensor[snr_idx].cali_flag);
            } else {

                // Start readings
                int adc_reading = cosmos_sensor_adc1_multisampling(&pSensor[snr_idx]);

                // Convert ADC reading to calibrated voltage
                adc_cali_raw_to_voltage(pSensor[snr_idx].snr_handle, adc_reading, &pSensor[snr_idx].reading);
            }
        } else {

            // Check sensor calibration
            if (pSensor[snr_idx].cali_flag == false) {

                // Sensor calibration
                cosmos_sensor_adc_cali_init(ADC_UNIT_2, pSensor[snr_idx].snr_chn, &pSensor[snr_idx].snr_handle, &pSensor[snr_idx].cali_flag);
            } else {

                // Get calibrated readings
                adc_oneshot_get_calibrated_result(adc2_handle, pSensor[snr_idx].snr_handle, pSensor[snr_idx].snr_chn, &pSensor[snr_idx].reading);
            }
        }
    }
}