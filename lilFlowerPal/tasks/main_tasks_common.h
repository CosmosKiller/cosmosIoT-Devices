#ifndef MAIN_TASKS_COMMON_H_
#define MAIN_TASKS_COMMON_H_

// BME680 task
#define BME680_TASK_STACK_SIZE 6144
#define BME680_TASK_PRIORITY   3
#define BME680_TASK_CORE_ID    1

// Encoder task
#define ENCODER_TASK_STACK_SIZE 2048
#define ENCODER_TASK_PRIORITY   10
#define ENCODER_TASK_CORE_ID    1

// Soil moisture task
#define SOIL_MOISTURE_TASK_STACK_SIZE 4096
#define SOIL_MOISTURE_TASK_PRIORITY   3
#define SOIL_MOISTURE_TASK_CORE_ID    0

// Pump init task
#define PUMP_INIT_TASK_STACK_SIZE 4096
#define PUMP_INIT_TASK_PRIORITY   4
#define PUMP_INIT_TASK_CORE_ID    0

#endif /* MAIN_TASKS_COMMON_H_ */