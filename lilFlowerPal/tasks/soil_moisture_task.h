#ifndef MAIN_SOIL_MOISTURE_TASK_H_
#define MAIN_SOIL_MOISTURE_TASK_H_

#include <driver/gpio.h>
#include <esp_err.h>

#include <cosmos_sensor.h>

#define SM_QTY 4

// Sensor 1, 2, 3 and 4 GPIO and ADC channel definitions
#define SM1_GPIO GPIO_NUM_34 /*!< GPIO pin for sensor 1 */
#define SM2_GPIO GPIO_NUM_35 /*!< GPIO pin for sensor 2 */
#define SM3_GPIO GPIO_NUM_32 /*!< GPIO pin for sensor 3 */
#define SM4_GPIO GPIO_NUM_33 /*!< GPIO pin for sensor 4 */

#define SM1_CHN ADC_CHANNEL_6 /*!< ADC channel for sensor 1 */
#define SM2_CHN ADC_CHANNEL_7 /*!< ADC channel for sensor 2 */
#define SM3_CHN ADC_CHANNEL_4 /*!< ADC channel for sensor 3 */
#define SM4_CHN ADC_CHANNEL_5 /*!< ADC channel for sensor 4 */

using sm_sensor_cb_t = void (*)(uint16_t endpoint_id, float value, void *user_data);

typedef struct {
    sm_sensor_cb_t cb = NULL;    /*!< This callback functon will be called periodically to report the temperature.*/
    uint16_t endpoint_id;        /*!< Endpoint_id associated with temperature sensor */
    void *user_data = NULL;      /*!< User data*/
    uint32_t interval_ms = 5000; /*!< Polling interval in milliseconds, defaults to 5000 ms */
} sm_sensor_config_t;

/**
 * @brief Configures the sensors and
 * initiate the reading routine
 */
esp_err_t soil_moisture_task_sensor_init(sm_sensor_config_t *pConfig, cosmos_sensor_t *pSensor);

#endif /* MAIN_SOIL_MOISTURE_TASK_H_ */