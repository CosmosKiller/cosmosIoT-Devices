#ifndef PUMP_TASK_H_
#define PUMP_TASK_H_

#define PUMP0 GPIO_NUM_25
#define PUMP1 GPIO_NUM_26
#define PUMP2 GPIO_NUM_27
#define PUMP3 GPIO_NUM_14

typedef void *pump_task_handle_t;

/**
 * @brief Configuration structure for the pump
 *
 */
typedef struct {
    uint16_t endpoint_id; /* Endpoint ID associated with the pump */
    gpio_num_t gpio;      /* GPIO pin associated with the pump */
} pump_task_config_t;

/**
 * @brief Configures irrigation pumps
 * service
 */
void pump_init_start(void);

#endif /* PUMP_TASK_H_ */