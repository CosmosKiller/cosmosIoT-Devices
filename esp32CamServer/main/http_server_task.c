#include <esp_log.h>

#include <http_server_task.h>
#include <http_stream_task.h>

#define TAG "http_server_task"

httpd_handle_t server;

httpd_handle_t http_server_task_start(httpd_handle_t server)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t stream_uri = {
            .uri = "/stream",
            .method = HTTP_GET,
            .handler = http_stream_task_handler,
            .user_ctx = NULL,
        };
        httpd_register_uri_handler(server, &stream_uri);
    }
    return server;
}