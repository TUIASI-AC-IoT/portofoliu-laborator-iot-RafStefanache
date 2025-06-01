#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_http_server.h"

static const char *TAG = "REST_SIM";

esp_err_t sensor_get_handler(httpd_req_t *req) {
    const char *response = "{\"sensor_id\":\"1\", \"value\":22.5}";
    httpd_resp_send(req, response, strlen(response));
    return ESP_OK;
}

esp_err_t sensor_post_handler(httpd_req_t *req) {
    char content[100];
    httpd_req_recv(req, content, req->content_len);
    ESP_LOGI(TAG, "Received config: %s", content);

    httpd_resp_set_status(req, "201 Created");
    httpd_resp_send(req, "{\"message\":\"Config created\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t sensor_put_handler(httpd_req_t *req) {
    char content[100];
    httpd_req_recv(req, content, req->content_len);
    ESP_LOGI(TAG, "Updated config: %s", content);

    httpd_resp_send(req, "{\"message\":\"Config updated\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

httpd_uri_t sensor_get_uri = {
    .uri       = "/sensor/1",
    .method    = HTTP_GET,
    .handler   = sensor_get_handler
};

httpd_uri_t sensor_post_uri = {
    .uri       = "/sensor/1",
    .method    = HTTP_POST,
    .handler   = sensor_post_handler
};

httpd_uri_t sensor_put_uri = {
    .uri       = "/sensor/1/config.json",
    .method    = HTTP_PUT,
    .handler   = sensor_put_handler
};

void start_rest_server() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &sensor_get_uri);
        httpd_register_uri_handler(server, &sensor_post_uri);
        httpd_register_uri_handler(server, &sensor_put_uri);
    }
}
