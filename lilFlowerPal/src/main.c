/**
 * @file main.c
 * @author Marcel Nahir Samur (mnsamur2014@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-06-09
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "bme680_task.h"
#include "soil_moisture_task.h"

void app_main(void)
{
    // bme680_task_start();
    soil_moisture_task_start();
}