#include <esp_camera.h>
#include <esp_log.h>
#include <esp_timer.h>

#include <http_stream_task.h>

#define TAG "http_stream_task"

#define PART_BOUNDARY "123456789000000000000987654321"
static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %zu\r\n\r\n";

// Stream control flag
static bool stream_enabled = false;

void http_stream_task_service_enabled(bool enable)
{
    ESP_LOGI(TAG, "Stream %s", enable ? "enabled" : "disabled");
    stream_enabled = enable;
}

bool http_stream_task_service_check(void)
{
    return stream_enabled;
}

esp_err_t http_stream_task_handler(httpd_req_t *pReq)
{
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    size_t jpg_buf_len = 0;
    uint8_t *jpg_buf = NULL;
    char part_buf[64];
    static int64_t last_frame = 0;
    if (!last_frame) {
        last_frame = esp_timer_get_time();
    }

    // Check if streaming is enabled
    if (!stream_enabled) {
        return httpd_resp_send_404(pReq);
    }

    res = httpd_resp_set_type(pReq, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK) {
        return res;
    }

    while (stream_enabled) {
        fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGE(TAG, "Camera capture failed");
            res = ESP_FAIL;
            break;
        }
        if (fb->format != PIXFORMAT_JPEG) {
            bool jpeg_converted = frame2jpg(fb, 80, &jpg_buf, &jpg_buf_len);
            if (!jpeg_converted) {
                ESP_LOGE(TAG, "JPEG compression failed");
                esp_camera_fb_return(fb);
                res = ESP_FAIL;
                break;
            }
        } else {
            jpg_buf_len = fb->len;
            jpg_buf = fb->buf;
        }

        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(pReq, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if (res == ESP_OK) {
            int hlen = snprintf(part_buf, sizeof(part_buf), _STREAM_PART, jpg_buf_len);
            if (hlen < 0 || hlen >= sizeof(part_buf)) {
                ESP_LOGE(TAG, "Header truncated (%d bytes needed >= %zu buffer)",
                         hlen, sizeof(part_buf));
                res = ESP_FAIL;
            } else {
                res = httpd_resp_send_chunk(pReq, part_buf, (size_t)hlen);
            }
        }
        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(pReq, (const char *)jpg_buf, jpg_buf_len);
        }
        if (fb->format != PIXFORMAT_JPEG) {
            free(jpg_buf);
        }
        esp_camera_fb_return(fb);
        if (res != ESP_OK) {
            break;
        }
        int64_t fr_end = esp_timer_get_time();
        int64_t frame_time = fr_end - last_frame;
        last_frame = fr_end;
        frame_time /= 1000;
        float fps = frame_time > 0 ? 1000.0f / (float)frame_time : 0.0f;
        ESP_LOGI(TAG, "MJPG: %luKB %lums (%.1ffps)", (uint32_t)(jpg_buf_len / 1024), (uint32_t)frame_time, fps);
    }

    last_frame = 0;
    return res;
}