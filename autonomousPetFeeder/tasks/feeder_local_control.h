#ifndef MAIN_FEEDER_LOCAL_CONTROL_H_
#define MAIN_FEEDER_LOCAL_CONTROL_H_

/**
 * @brief Pass the value as the ulValue
 * in the xTaskNotifyFromISR() function
 * to set the notification type.
 *
 * Value will be evalueted within the
 * feeder local control task
 */
typedef enum {
    NOTIFY_TYPE_FLOWER_PUMP = 0, /*!< Initiates the irrigation system. */
    NOTIFY_TYPE_WATER_PUMP,      /*!< Fills the water bowl. */
    NOTIFY_TYPE_PET_FEEDER,      /*!< Serves a food portion. */
    NOTIFY_TYPE_PET_DEFAULT,     /*!< Default value, no action is done. */
} feeder_local_control_notify_type_e;

/**
 * @brief Configures the buttons ISR, so
 * that you can locally control the feeder's
 * funtions.
 */
void feeder_local_control_start(void);

#endif /* MAIN_FEEDER_LOCAL_CONTROL_H_ */