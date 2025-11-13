#ifndef HTTP_SERVER_TASK_H
#define HTTP_SERVER_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esp_http_server.h>

/**
 * @brief Set up and start HTTP server
 *
 * @param server Existing server handle or NULL
 * @return httpd_handle_t New server handle
 */
httpd_handle_t http_server_task_start(httpd_handle_t server);

#ifdef __cplusplus
}
#endif

#endif // HTTP_SERVER_TASK_H