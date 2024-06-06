/**
 * @file main.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2023-03-23
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "feeder_local_control.h"

static void led_blink_task(void *pvParameter)
{
    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    xTaskCreatePinnedToCore(&led_blink_task, "led_blink_task", 4096, NULL, 5, NULL, 0);
    feeder_local_control_start();
}