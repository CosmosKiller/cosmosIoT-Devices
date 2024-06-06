#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "cosmos_pump.h"

// Handle for cosmos_pump_begin
static bool s_pump_begin_handle = false;

/**
 * @brief Pump setup function
 *
 * @param pPump Pointer to the strutct that contains all of the info
 * about the pumps used in the project
 */
static void cosmos_pump_begin(cosmos_pump_t *pPump)
{
    int pin_idx;
    // Bit mask is stored as an ull variable
    unsigned long long my_bit_mask;

    // Using bitwise OR we build the bit mask
    for (pin_idx = 0; pin_idx < 3; pin_idx++) {
        if (pin_idx == 0) {
            /*
             * As the pump relay is connected to only one GPIO (on pin_idx=0),
             * we apply OR between that defined pin and the first pin on the rgb led.
             *
             * We store the result in our previously defined variable.
             */
            my_bit_mask = BIT(pPump->pPumpInfo->pin[pin_idx]) | BIT(pPump->pLedInfo[pin_idx]);
        } else {
            /*
             * For the remainig rgb led pins (pin_idx 1 and 2),
             * we apply OR between the value stored in our variable
             * and the the remaining led pins.
             */
            my_bit_mask = my_bit_mask | BIT(pPump->pLedInfo[pin_idx]);
        }
    }

    // Initialize GPIO config struct
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = my_bit_mask,
    };

    // Configure GPIO with the given settings
    gpio_config(&io_conf);

    /*
     * Set the gpio pin level to the innital state,
     * defined in the cosmos_devices_t struct
     * of the main.c file
     */
    gpio_set_level(pPump->pPumpInfo->pin[0], pPump->pPumpInfo->state);
    gpio_set_level(pPump->pLedInfo[0], 0);
    gpio_set_level(pPump->pLedInfo[1], 0);
    gpio_set_level(pPump->pLedInfo[2], 0);

    s_pump_begin_handle = true;
}

/**
 * @brief Simple loop used for reseting the pump state/values
 *
 * @param pPump Pointer to the strutct that contains all of the info
 * about the pumps used in the project
 */
static void cosmos_pump_power_off(cosmos_pump_t *pPump)
{
    // Reset pump state
    gpio_set_level(pPump->pPumpInfo->pin[0], 0);
    pPump->pPumpInfo->state = 0;

    // Reset pump's LED state
    for (int i = 0; i < 3; i++)
        gpio_set_level(pPump->pLedInfo[i], 0);
}

void cosmos_pump_control(char *pTopic, cosmos_pump_t *pPump, int engage, cosmos_sensor_data_t *pSensorData)
{
    // Check if the pump is configured
    if (s_pump_begin_handle == false) {
        cosmos_pump_begin(pPump);
    }

    char *pRet = strstr(pTopic, pPump->pPumpInfo->sn);
    /*
     * If the topic contains the
     * serial number stored in the
     * pPump struct, excution will
     * continue. Otherwise, the
     * function es exited
     */
    if (pRet) {
        int wl = 0, sm = 0;
        float temp = 0, hum = 0;

// Samples are only for defined sensors
#ifdef SNRWL_PIN
        wl = cosmos_pump_ac1_multisampling(&pPump->snr_wl);
        wl = COSMOS_MAP(wl, 0, 4095, 0, 100);
#endif

#ifdef SNRSM_PIN
        sm = cosmos_pump_ac1_multisampling(&pPump->snr_sm);
        sm = COSMOS_MAP(sm, 0, 4095, 0, 100);
#endif

#ifdef SNRTH_PIN
        temp = myDHT.getTemperature();
        hum = myDHT.getHumidity();
#endif

        /*
         * When trying to engage the pump, it must be verified
         * that the sensor readings are within the established
         * thresholds.
         *
         * Furthermore, if pump is already activated and values
         * ​comes out of the thresholds, it'll be automatically
         * disengaged.
         *
         * Also, in order to avoid getting an odd be behaviour
         * values mustn't be NaN.
         */
        if (wl > pPump->pSnrInfo->thr_min && wl < pPump->pSnrInfo->thr_max && !isnan((float)wl)) {
            cosmos_pump_power_off(pPump);
            // cosmosMqttPublish("0", pPump->pPumpInfo->sn, RX_STATE);
            // cosmosMqttPublish("Atención! Nivel de agua crítico", pPump->pPumpInfo->sn, RX_CONTROL);
            gpio_set_level(pPump->pLedInfo[0], 1);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            gpio_set_level(pPump->pLedInfo[0], 0);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        } else if (sm > pPump->snr_sm.thr_lvl && !isnan((float)sm)) {
            cosmos_pump_power_off(pPump);
            // cosmosMqttPublish("0", pPump->pPumpInfo->sn, RX_STATE);
            // cosmosMqttPublish("Atención! Nivel de humedad del suelo crítico", pPump->pPumpInfo->sn, RX_CONTROL);
            gpio_set_level(pPump->pLedInfo[0], 1);
            gpio_set_level(pPump->pLedInfo[2], 1);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            gpio_set_level(pPump->pLedInfo[0], 0);
            gpio_set_level(pPump->pLedInfo[2], 0);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        } else if (temp > pPump->snr_th.thr_lvl && !isnan(temp)) {
            cosmos_pump_power_off(pPump);
            // cosmosMqttPublish("0", pPump->pPumpInfo->sn, RX_STATE);
            // cosmosMqttPublish("Atención! Nivel de temperatura crítico", pPump->pPumpInfo->sn, RX_CONTROL);
            gpio_set_level(pPump->pLedInfo[0], 1);
            gpio_set_level(pPump->pLedInfo[1], 1);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            gpio_set_level(pPump->pLedInfo[0], 0);
            gpio_set_level(pPump->pLedInfo[1], 0);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }

        /*
         * Once the sensed values reached the
         * desired threshold, pump will be ready
         * to be engaged
         */
        if (wl >= pPump->snr_wl.thr_lvl && temp <= pPump->snr_th.thr_lvl && sm <= pPump->snr_sm.thr_lvl) {
            switch (engage) {

            case ENGAGE_OK:
                gpio_set_level(pPump->pPumpInfo->pin[0], 1);
                pPump->pPumpInfo->state = 1;
                // cosmosMqttPublish("1", pPump->pPumpInfo->sn, RX_STATE);
                // cosmosMqttPublish("Sistema iniciado...", pPump->pPumpInfo->sn, RX_CONTROL);
                gpio_set_level(pPump->pLedInfo[2], 0);
                gpio_set_level(pPump->pLedInfo[1], 1);
                gpio_set_level(pPump->pLedInfo[0], 0);
                vTaskDelay(2000 / portTICK_PERIOD_MS);
                break;

            case ENGAGE_RN:
                gpio_set_level(pPump->pLedInfo[1], 1);
                // cosmosMqttPublish("Sistema en marcha OK!", pPump->pPumpInfo->sn, RX_CONTROL);
                vTaskDelay(2000 / portTICK_PERIOD_MS);
                break;

            case ENGAGE_NO:
                gpio_set_level(pPump->pLedInfo[2], 1);
                // cosmosMqttPublish("Sistema en espera.", pPump->pPumpInfo->sn, RX_CONTROL);
                vTaskDelay(2000 / portTICK_PERIOD_MS);
                break;

            default:
                break;
            }
        }
        pSensorData->wl_data = wl;
        pSensorData->sm_data = sm;
        pSensorData->tm_data = temp;
        pSensorData->hm_data = hum;
    }
}