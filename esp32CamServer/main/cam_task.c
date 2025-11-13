#include <esp_camera.h>
#include <esp_log.h>
#include <esp_psram.h>

#include <cam_task.h>

#define TAG "cam_task"

esp_err_t cam_task_init()
{
    bool has_psram = esp_psram_is_initialized();
    ESP_LOGI(TAG, "PSRAM %s", has_psram ? "present" : "not present");

    camera_config_t config = {
        .pin_pwdn = PWDN_GPIO_NUM,
        .pin_reset = RESET_GPIO_NUM,
        .pin_xclk = XCLK_GPIO_NUM,
        .pin_sccb_sda = SIOD_GPIO_NUM,
        .pin_sccb_scl = SIOC_GPIO_NUM,
        .pin_d7 = Y9_GPIO_NUM,
        .pin_d6 = Y8_GPIO_NUM,
        .pin_d5 = Y7_GPIO_NUM,
        .pin_d4 = Y6_GPIO_NUM,
        .pin_d3 = Y5_GPIO_NUM,
        .pin_d2 = Y4_GPIO_NUM,
        .pin_d1 = Y3_GPIO_NUM,
        .pin_d0 = Y2_GPIO_NUM,
        .pin_vsync = VSYNC_GPIO_NUM,
        .pin_href = HREF_GPIO_NUM,
        .pin_pclk = PCLK_GPIO_NUM,

        .xclk_freq_hz = 20000000,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,
        .pixel_format = PIXFORMAT_JPEG,
        .frame_size = FRAMESIZE_VGA,
        .jpeg_quality = 12,
        .fb_count = has_psram ? 2 : 1, // only use 2 FB if PSRAM present
        .fb_location = has_psram ? CAMERA_FB_IN_PSRAM : CAMERA_FB_IN_DRAM,
    };

    ESP_LOGI(TAG, "Camera config: xclk=%u, frame=%d, fb_count=%d, fb_loc=%s",
             config.xclk_freq_hz, config.frame_size, config.fb_count,
             config.fb_location == CAMERA_FB_IN_PSRAM ? "PSRAM" : "DRAM");

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        return err;
    }

    return ESP_OK;
}