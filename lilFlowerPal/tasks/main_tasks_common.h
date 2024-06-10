#ifndef MAIN_TASKS_COMMON_H_
#define MAIN_TASKS_COMMON_H_

// LED blink task
#define BME680_TASK_STACK_SIZE 6144
#define BME680_TASK_PRIORITY   3
#define BME680_TASK_CORE_ID    1

// Encoder task
#define ENCODER_TASK_STACK_SIZE 2048
#define ENCODER_TASK_PRIORITY   10
#define ENCODER_TASK_CORE_ID    0

#endif /* MAIN_TASKS_COMMON_H_ */