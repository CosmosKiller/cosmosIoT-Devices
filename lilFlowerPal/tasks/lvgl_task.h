#ifndef MAIN_LVGL_TASK_H_
#define MAIN_LVGL_TASK_H_

#include <driver/gpio.h>
#include <driver/spi_master.h>

#include <main_tasks_common.h>

// LCD pin definition
#define LCD_PIN_MOSI 23
#define LCD_PIN_CLK  18
#define LCD_PIN_DC   2
#define LCD_PIN_RST  19
#define LCD_PIN_ND   -1

// LCD interface definitions
#define LCD_HOST           SPI3_HOST
#define LCD_PIXEL_CLK_HZ   (26 * 1000 * 1000)
#define LCD_CMD_BITS       8
#define LCD_PARAM_BITS     8
#define LCD_SPI_MODE       3
#define LCD_BITS_PER_PIXEL 16
#define LCD_H_RES          240
#define LCD_V_RES          240

/**
 * @brief Notify LVGL task that device is commissioned
 *
 */
void lvgl_task_device_commissioned(void);

/**
 * @brief Init LCD HW, LVGL and sets main screen
 *
 */
void lvgl_task_start(void);

#endif /* MAIN_LVGL_TASK_H_ */