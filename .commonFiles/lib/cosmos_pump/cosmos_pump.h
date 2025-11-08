#ifndef MAIN_COSMOS_PUMP_H_
#define MAIN_COSMOS_PUMP_H_

#include "cosmos_devices.h"
#include "cosmos_sensor.h"

/**
 * @brief States of the pump
 *
 */
typedef enum {
    ENGAGE_OK = 0, /*!< Pump started */
    ENGAGE_RN,     /*!< Pump running */
    ENGAGE_NO,     /*!< Pump in standby */
} cosmos_pump_state_e;

/**
 * @brief Use this struct to define all the parameters
 * that needs to be checked in order to get the pump
 * going
 *
 * @note If you are going to do without any of the sensors,
 * just create an arry of zeros -> {0, 0}
 */
typedef struct {
    cosmos_devices_t *pPumpInfo;    /*<! Pump relay serial number, pins and state */
    const int *pLedInfo;            /*!< Pins in which the pump's LED is connected */
    cosmos_sensor_info_t *pSnrInfo; /*!< Sensor information */
} cosmos_pump_t;

/**
 * @brief Call this function whenever you want to check the
 * status of the pump
 *
 * @param pTopic Incoming topic from the MQTT broker
 * @param pPump Pointer to the struct that defines the pump
 * @param engage
 *          - If you want to turn on the pump ENGAGE_OK ;
 *          - If you want to leave it in idle ENGAGE_NO ;
 *          - If pump is already running ENGAGE_RN ;
 * @param pSensorData Pointer to the struct in which the sensed data will be stored
 */
void cosmos_pump_control(char *pTopic, cosmos_pump_t *pPump, int engage, cosmos_sensor_data_t *pSensorData);

#endif /* MAIN_COSMOS_PUMP_H_ */