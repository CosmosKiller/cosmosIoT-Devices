#ifndef MAIN_TASKS_COMMON_H_
#define MAIN_TASKS_COMMON_H_

// LED blink task
#define LED_BLINK_TASK_STACK_SIZE 4096
#define LED_BLINK_TASK_PRIORITY   4
#define LED_BLINK_TASK_CORE_ID    0

// MQ135 read task
#define MQ135_READ_TASK_STACK_SIZE 4096
#define MQ135_READ_TASK_PRIORITY   5
#define MQ135_READ_TASK_CORE_ID    1

// DHT22 read task
#define DHT22_READ_TASK_STACK_SIZE 4096
#define DHT22_READ_TASK_PRIORITY   5
#define DHT22_READ_TASK_CORE_ID    1

// PIR sensor task
#define PIR_SENSOR_TASK_STACK_SIZE 4096
#define PIR_SENSOR_TASK_PRIORITY   4
#define PIR_SENSOR_TASK_CORE_ID    0

// Encoder task
#define ENCODER_TASK_STACK_SIZE 2048
#define ENCODER_TASK_PRIORITY   10
#define ENCODER_TASK_CORE_ID    0

#endif /* MAIN_TASKS_COMMON_H_ */