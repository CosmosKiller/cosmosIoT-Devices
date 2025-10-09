/**
 * @file lvgl_task.cpp
 * @author Marcel Nahir Samur (mnsamur2014@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-10-05
 *
 * @copyright Copyright (c) 2025
 *
 */

// Include ESP-IDF libraries
#include <esp_err.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Include LVGL libraries
#include <esp_lvgl_port.h>
#include <lvgl.h>

// Include ST7789 libraries
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_vendor.h>

#include <lvgl_task.h>

static const char *TAG = "lvgl_task";

static esp_lcd_panel_io_handle_t lcd_io = NULL;
static esp_lcd_panel_handle_t lcd_panel = NULL;

extern lv_obj_t *screens[4];
static lv_display_t *lvgl_disp = NULL;

/**
 * @brief LCD Hardware initialization
 *
 * @return esp_err_t
 */
static esp_err_t lvgl_task_lcd_init(void)
{
    esp_err_t ret = ESP_OK;

    // SPI bus init
    ESP_LOGD(TAG, "Initialize SPI bus");
    const spi_bus_config_t buscfg = {
        .sclk_io_num = LCD_PIN_CLK,
        .mosi_io_num = LCD_PIN_MOSI,
        .miso_io_num = LCD_PIN_ND,
        .quadwp_io_num = LCD_PIN_ND,
        .quadhd_io_num = LCD_PIN_ND,
        .max_transfer_sz = LCD_H_RES * 80 * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // LCD IO config
    ESP_LOGD(TAG, "Install panel IO");
    const esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = LCD_PIN_DC,
        .cs_gpio_num = LCD_PIN_ND,
        .pclk_hz = LCD_PIXEL_CLK_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = LCD_SPI_MODE,
        .trans_queue_depth = 10,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &lcd_io));

    // LCD driver init
    ESP_LOGD(TAG, "Install LCD driver");
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_PIN_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = LCD_BITS_PER_PIXEL,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(lcd_io, &panel_config, &lcd_panel));

    // Default states for ST7789
    esp_lcd_panel_reset(lcd_panel);
    esp_lcd_panel_init(lcd_panel);
    esp_lcd_panel_set_gap(lcd_panel, 0, 80);
    esp_lcd_panel_mirror(lcd_panel, false, false);
    esp_lcd_panel_disp_on_off(lcd_panel, true);
    esp_lcd_panel_invert_color(lcd_panel, false);

    return ret;
}

/**
 * @brief LVGL initialization
 *
 * @return esp_err_t
 */
static esp_err_t lvgl_task_lvgl_init(void)
{
    // LVGL init
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

    // Add ST7789 LCD screen
    ESP_LOGD(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = lcd_io,
        .panel_handle = lcd_panel,
        .buffer_size = LCD_H_RES * 40,
        .double_buffer = 1,
        .hres = LCD_H_RES,
        .vres = LCD_V_RES,
        .monochrome = false,
#if LVGL_VERSION_MAJOR >= 9
        .color_format = LV_COLOR_FORMAT_RGB565,
#endif
        .rotation = {
            .swap_xy = false,
            .mirror_x = true,
            .mirror_y = true,
        },
        .flags = {
            .buff_dma = true,
#if LVGL_VERSION_MAJOR >= 9
            .swap_bytes = true,
#endif
        },
    };
    lvgl_disp = lvgl_port_add_disp(&disp_cfg);

    return ESP_OK;
}

static void lvgl_task_main_display(void)
{
    // Task lock
    lvgl_port_lock(0);

    // TODO: LVGL objects code here

    // Screen 1
    screens[0] = lv_obj_create(NULL);
    lv_obj_t *label1 = lv_label_create(screens[0]);
    lv_label_set_text(label1, "#ff00eaff"
                              "Screen 1");
    lv_obj_center(label1);

    // Screen 2
    screens[1] = lv_obj_create(NULL);
    lv_obj_t *label2 = lv_label_create(screens[1]);
    lv_label_set_text(label2, "#003cffff"
                              "Screen 2");
    lv_obj_center(label2);

    // Screen 3
    screens[2] = lv_obj_create(NULL);
    lv_obj_t *label3 = lv_label_create(screens[2]);
    lv_label_set_text(label3, "#7bff00ff"
                              "Screen 3");
    lv_obj_center(label3);

    // Screen 4
    screens[3] = lv_obj_create(NULL);
    lv_obj_t *label4 = lv_label_create(screens[3]);
    lv_label_set_text(label4, "#FF0000"
                              "Screen 4");
    lv_obj_center(label4);

    // Load first screen
    lv_scr_load(screens[0]);

    // Task unlock
    lvgl_port_unlock();
}

void lvgl_task_start(void)
{
    ESP_ERROR_CHECK(lvgl_task_lcd_init());

    ESP_ERROR_CHECK(lvgl_task_lvgl_init());

    lvgl_task_main_display();
}