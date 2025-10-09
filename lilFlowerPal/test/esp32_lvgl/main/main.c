#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_lvgl_port.h"
#include "lvgl.h"

// For ST7789
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"

// --- PIN CONFIG (adjust to your wiring) ---
#define PIN_LCD_MOSI 23
#define PIN_LCD_CLK  18
#define PIN_LCD_DC   2
#define PIN_LCD_RST  19

#define LCD_HOST           SPI3_HOST
#define LCD_PIXEL_CLK_HZ   (26 * 1000 * 1000) // 40 MHz
#define LCD_CMD_BITS       8
#define LCD_PARAM_BITS     8
#define LCD_SPI_MODE       3
#define LCD_BITS_PER_PIXEL 16 // RGB565

// Encoder pins (example)
#define PIN_ENCODER_A   GPIO_NUM_4
#define PIN_ENCODER_B   GPIO_NUM_17
#define PIN_ENCODER_BTN GPIO_NUM_16

// LCD resolution
#define LCD_H_RES 240
#define LCD_V_RES 240

static const char *TAG = "LCD_EXAMPLE";

// LVGL image declare
LV_IMG_DECLARE(esp_logo)

/* LCD IO and panel */
static esp_lcd_panel_io_handle_t lcd_io = NULL;
static esp_lcd_panel_handle_t lcd_panel = NULL;

/* LVGL display and encoder */
static lv_obj_t *screens[4];
static int current_screen = 0;
static lv_display_t *lvgl_disp = NULL;

static esp_err_t app_lcd_init(void)
{
    esp_err_t ret = ESP_OK;

    /* LCD initialization */
    ESP_LOGD(TAG, "Initialize SPI bus");
    const spi_bus_config_t buscfg = {
        .sclk_io_num = PIN_LCD_CLK,
        .mosi_io_num = PIN_LCD_MOSI,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_H_RES * 80 * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    ESP_LOGD(TAG, "Install panel IO");
    const esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = PIN_LCD_DC,
        .cs_gpio_num = -1,
        .pclk_hz = LCD_PIXEL_CLK_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = LCD_SPI_MODE,
        .trans_queue_depth = 10,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &lcd_io));

    ESP_LOGD(TAG, "Install LCD driver");
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = LCD_BITS_PER_PIXEL,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(lcd_io, &panel_config, &lcd_panel));

    esp_lcd_panel_reset(lcd_panel);
    esp_lcd_panel_init(lcd_panel);
    esp_lcd_panel_set_gap(lcd_panel, 0, 80);
    esp_lcd_panel_mirror(lcd_panel, false, false);
    esp_lcd_panel_disp_on_off(lcd_panel, true);
    esp_lcd_panel_invert_color(lcd_panel, false);

    return ret;
}

static esp_err_t app_lvgl_init(void)
{
    /* Initialize LVGL */
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 4,       /* LVGL task priority */
        .task_stack = 4096,       /* LVGL task stack size */
        .task_affinity = -1,      /* LVGL task pinned to core (-1 is no affinity) */
        .task_max_sleep_ms = 500, /* Maximum sleep in LVGL task */
        .timer_period_ms = 5      /* LVGL timer tick period in ms */
    };
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

    /* Add LCD screen */
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

static void switch_screen(int idx)
{
    if (idx < 0)
        idx = 3;
    if (idx > 3)
        idx = 0;

    current_screen = idx;
    lv_scr_load(screens[current_screen]);
    ESP_LOGI(TAG, "Switched to screen %d", current_screen);
}

static void app_main_display(void)
{
    /* Task lock */
    lvgl_port_lock(0);

    /* Your LVGL objects code here .... */
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

    /* Task unlock */
    lvgl_port_unlock();
}

void app_main(void)
{
    /* LCD HW initialization */
    ESP_ERROR_CHECK(app_lcd_init());

    /* LVGL initialization */
    ESP_ERROR_CHECK(app_lvgl_init());

    /* Show LVGL objects */
    app_main_display();

    switch_screen(0);

    // --- LVGL loop ---
    ESP_LOGI(TAG, "App running, rotate encoder to navigate, press to change screen!");

    while (1) {
        for (int i = 0; i < 4; i++) {
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            switch_screen(i);
        }
    }
}