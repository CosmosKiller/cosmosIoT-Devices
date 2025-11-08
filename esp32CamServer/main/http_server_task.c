#include "esp_log.h"

#include "http_server_task.h"
#include "http_stream_task.h"

#define TAG "http_server_task"

extern httpd_handle_t server;

httpd_handle_t start_webserver(httpd_handle_t server)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t stream_uri = {
            .uri = "/stream",
            .method = HTTP_GET,
            .handler = stream_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(server, &stream_uri);
    }
    return server;
}

void stop_webserver(httpd_handle_t server)
{
    if (server) {
        httpd_stop(server);
    }
}