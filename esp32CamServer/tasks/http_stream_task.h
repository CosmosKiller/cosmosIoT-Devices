#ifndef HTTP_STREAM_TASK_H
#define HTTP_STREAM_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <esp_err.h>
#include <esp_http_server.h>

/**
 * @brief HTTP stream task handler
 *
 * @param pReq Pointer to HTTP request structure
 * @return esp_err_t
 */
esp_err_t http_stream_task_handler(httpd_req_t *pReq);

/**
 * @brief Enable/disable streaming without stopping server
 *
 * @param enable true to enable stream, false to disable
 */
void http_stream_task_service_enabled(bool enable);

/**
 * @brief Check if streaming is enabled
 *
 * @return true if streaming enabled, false otherwise
 */
bool http_stream_task_service_check(void);

#ifdef __cplusplus
}
#endif

#endif // HTTP_STREAM_TASK_H