#ifndef MAIN_COSMOS_SOCKET_H_
#define MAIN_COSMOS_SOCKET_H_

#include "cosmos_devices.h"

/**
 * @brief This function is used to controll the state (on/off)
 * of a certain socket
 *
 * @param sn_value Serial number used to compare and
 * determine with which socket we are trying to interact
 * @param pSocket Pointer to the strutct that contains all of the info
 * about the sockets used in the project
 * @param qty Quantity of sockets used in the project
 */
void cosmos_socket_control(const char *sn_value, cosmos_devices_t *pSocket, size_t qty);

#endif /* MAIN_COSMOS_SOCKET_H_ */