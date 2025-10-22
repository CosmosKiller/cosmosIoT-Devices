#ifndef MAIN_COSMOS_POWER_H_
#define MAIN_COSMOS_POWER_H_

#include "cosmos_devices.h"

/**
 * @brief This function is used to controll the state (on/off)
 * of a certain power device
 *
 * @param sn_value Serial number used to compare and
 * determine with which power device we are trying to interact
 * @param pPower Pointer to the strutct that contains all of the info
 * about the power devices used in the project
 * @param qty Quantity of power devices used in the project
 */
void cosmos_power_control(const char *sn_value, cosmos_devices_t *pPower, size_t qty);

#endif /* MAIN_COSMOS_POWER_H_ */