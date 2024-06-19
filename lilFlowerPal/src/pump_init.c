/**
 * @file pump_init.c
 * @author Marcel Nahir Samur (mnsamur2014@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-06-18
 *
 * @copyright Copyright (c) 2024
 *
 */
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "cosmos_devices.h"
#include "cosmos_power.h"
#include "main_tasks_common.h"

cosmos_devices_t pumps[] = {
    {
        .pin = {GPIO_NUM_25, 0, 0},
        .state = 0,
        .sn = "PWRDc-aaa0000",
    },
    {
        .pin = {GPIO_NUM_26, 0, 0},
        .state = 0,
        .sn = "PWRDc-aaa0001",
    },
    {
        .pin = {GPIO_NUM_27, 0, 0},
        .state = 0,
        .sn = "PWRDc-aaa0002",
    },
    {
        .pin = {GPIO_NUM_14, 0, 0},
        .state = 0,
        .sn = "PWRDc-aaa0003",
    },
};

/**
 * @brief Trigger moisture sensor readings
 *
 * @param pvParameters Parameter which can be passed to the task.
 */
static void pump_init(void *pvParameters)
{
    int dev_qty = QTY(pumps);
    while (1) {
        for (int idx = 0; idx < dev_qty; idx++) {
            cosmos_power_control(pumps[idx].sn, pumps, dev_qty);
            printf("Pump %d state: %d\n", idx, pumps[idx].state);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

void pump_init_start(void)
{
    xTaskCreatePinnedToCore(&pump_init, "pump_init", PUMP_INIT_TASK_STACK_SIZE, NULL, PUMP_INIT_TASK_PRIORITY, NULL, PUMP_INIT_TASK_CORE_ID);
}