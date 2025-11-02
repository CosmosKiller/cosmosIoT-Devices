#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "nvs_flash.h"

#include "cam_task.h"
#include "http_server_task.h"
#include "pir_detection_task.h"
#include "wifi_task.h"

static const char *TAG = "app_main";

void app_main()
{
    esp_err_t err = ESP_OK;

    // Robust NVS init
    err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS needs erase (err=%d). Erasing and retrying...", err);
        nvs_flash_erase();
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_flash_init failed: %d", err);
        return;
    }

    // Initialize the camera
    ESP_ERROR_CHECK(camera_init());

    // Start Wi-Fi
    wifi_init_sta();

    ESP_LOGI(TAG, "Starting PIR motion sensor app...");

    // Install ISR service ONCE for all GPIO-based modules
    gpio_install_isr_service(0);

    pir_detection_task_init();

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
