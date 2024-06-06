/**
 * @file main.c
 * @author Marcel Nahir Samur (mnsamur2014@gmail.com)
 * @brief application entry point
 * @version 0.1
 * @date 2023-03-21
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "bme680.h"

#define I2C_MASTER_SCL_IO 22    /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO 21    /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_0 /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 100000 /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define BME680_SENSOR_ADDR 0x77     /*!< BME680 I2C address */

static const char *TAG = "BME680";

/**
 * @brief i2c master initialization
 */
static esp_err_t i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
        // .clk_flags = 0,          /*!< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. */
    };
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) {
        return err;
    }
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

/**
 * @brief BME680 initialization
 */
static esp_err_t bme680_init_sensor(bme680_t *sensor)
{
    sensor->i2c_address = BME680_SENSOR_ADDR;
    sensor->i2c_port = I2C_MASTER_NUM;
    sensor->delay_ms = vTaskDelay;

    if (bme680_init(sensor) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize BME680 sensor");
        return ESP_FAIL;
    }

    // Set sensor settings
    bme680_set_sensor_settings(BME680_OST_SEL | BME680_OSH_SEL | BME680_OSP_SEL | BME680_FILTER_SEL, sensor);

    // Set sensor mode
    bme680_set_sensor_mode(BME680_FORCED_MODE, sensor);

    return ESP_OK;
}

void app_main(void)
{
    ESP_ERROR_CHECK(i2c_master_init());

    bme680_t sensor;
    if (bme680_init_sensor(&sensor) != ESP_OK) {
        ESP_LOGE(TAG, "BME680 initialization failed");
        return;
    }

    while (1) {
        // Trigger a new measurement
        bme680_set_sensor_mode(BME680_FORCED_MODE, &sensor);
        vTaskDelay(pdMS_TO_TICKS(sensor.delay_ms));

        // Read the sensor data
        struct bme680_field_data data;
        if (bme680_get_sensor_data(&data, &sensor) == ESP_OK) {
            ESP_LOGI(TAG, "Temperature: %.2f °C", data.temperature / 100.0);
            ESP_LOGI(TAG, "Humidity: %.2f %%", data.humidity / 1000.0);
            ESP_LOGI(TAG, "Pressure: %.2f hPa", data.pressure / 100.0);
            ESP_LOGI(TAG, "Gas resistance: %d ohms", data.gas_resistance);
        } else {
            ESP_LOGE(TAG, "Failed to get BME680 sensor data");
        }

        vTaskDelay(pdMS_TO_TICKS(2000)); // Delay between measurements
    }
}
