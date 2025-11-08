#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "toggle.h"
#include "wifi.h"

#define WIFI_SSID "Fibertel WiFi268 2.4GHz"
#define WIFI_PASS "0043669392"

extern "C" void app_main(void)
{
    // Init Wi-Fi
    wifi_init_sta(WIFI_SSID, WIFI_PASS);

    // Init toggle task
    toggle_start();

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}