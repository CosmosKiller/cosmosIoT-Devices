#ifndef PIR_SENSOR_H
#define PIR_SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/gpio.h"

#define PIR_PIN           GPIO_NUM_16
#define LED_PIN           GPIO_NUM_4
#define DETECTION_TIME_MS 5000 // 5 seconds

void pir_detection_task_init(void);

#ifdef __cplusplus
}
#endif

#endif // PIR_SENSOR_H