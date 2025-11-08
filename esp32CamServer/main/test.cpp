#include <esp_err.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <lib/support/CodeUtils.h>

#include <pir_sensor_task.h>

static const char *TAG = "pir_sensor_task";

typedef struct {
    pir_sensor_config_t *config;
    bool is_initialized;
    esp_timer_handle_t timer;
} pir_sensor_ctx_t;

static pir_sensor_ctx_t s_ctx;

static void pir_gpio_cb(void *arg)
{
    static bool occupancy = false;
    bool new_occupancy = gpio_get_level(PIR_PIN);

    auto *ctx = (pir_sensor_ctx_t *)arg;
    if (!(ctx && ctx->config)) {
        return;
    }

    // we only need to notify application layer if occupancy changed
    if (occupancy != new_occupancy) {
        gpio_set_level(LED_PIN, new_occupancy);
        occupancy = new_occupancy;
        if (ctx->config->cb) {
            ctx->config->cb(ctx->config->endpoint_id, new_occupancy, ctx->config->user_data);
        }
    }
}

static void pir_gpio_init(gpio_num_t pin)
{
    gpio_reset_pin(pin);
    gpio_set_intr_type(pin, GPIO_INTR_ANYEDGE);
    gpio_set_direction(pin, GPIO_MODE_INPUT);

    gpio_set_pull_mode(pin, GPIO_PULLDOWN_ONLY);

    // gpio_install_isr_service(0);
    // gpio_isr_handler_add(pin, pir_gpio_handler, NULL);
}

static void led_gpio_init(gpio_num_t pin)
{
    gpio_reset_pin(pin);
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);

    gpio_set_pull_mode(pin, GPIO_PULLDOWN_ONLY);
    gpio_set_level(pin, 0);
}

esp_err_t pir_sensor_init(pir_sensor_config_t *config)
{
    esp_err_t err;

    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (s_ctx.is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    pir_gpio_init(PIR_PIN);
    led_gpio_init(LED_PIN);

    // Keep the configuration and sensor parameters
    s_ctx.config = config;

    // Create a periodic timer to read the sensor
    const esp_timer_create_args_t read_args = {
        .callback = pir_gpio_cb,
        .arg = &s_ctx,
        .name = "pir_gpio_read",
    };

    err = esp_timer_create(&read_args, &s_ctx.timer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create timer for soil moisture sensor");
        return err;
    }

    // Start the timer to trigger every 5 seconds
    err = esp_timer_start_periodic(s_ctx.timer, 5000 * 1000);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start timer for soil moisture sensor");
        esp_timer_delete(s_ctx.timer);
        return err;
    }

    s_ctx.is_initialized = true;

    return ESP_OK;
}

// ----------------------------------------

#include <esp_log.h>
#include <lib/support/CodeUtils.h>

#include <pir_sensor_task.h>

typedef struct {
    pir_sensor_config_t *config;
    bool is_initialized;
} pir_sensor_ctx_t;

static pir_sensor_ctx_t s_ctx;

static void IRAM_ATTR pir_gpio_handler(void *arg)
{
    static bool occupancy = false;
    bool new_occupancy = gpio_get_level(PIR_PIN);

    // we only need to notify application layer if occupancy changed
    if (occupancy != new_occupancy) {
        gpio_set_level(LED_PIN, new_occupancy);
        occupancy = new_occupancy;
        if (s_ctx.config->cb) {
            s_ctx.config->cb(s_ctx.config->endpoint_id, new_occupancy, s_ctx.config->user_data);
        }
    }
}

static void pir_gpio_init(gpio_num_t pin)
{
    gpio_reset_pin(pin);
    gpio_set_intr_type(pin, GPIO_INTR_ANYEDGE);
    gpio_set_direction(pin, GPIO_MODE_INPUT);

    gpio_set_pull_mode(pin, GPIO_PULLDOWN_ONLY);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(pin, pir_gpio_handler, NULL);
}

static void led_gpio_init(gpio_num_t pin)
{
    gpio_reset_pin(pin);
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);

    gpio_set_pull_mode(pin, GPIO_PULLDOWN_ONLY);
    gpio_set_level(pin, 0);
}

esp_err_t pir_sensor_init(pir_sensor_config_t *config)
{
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (s_ctx.is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    pir_gpio_init(PIR_PIN);
    led_gpio_init(LED_PIN);

    s_ctx.config = config;
    return ESP_OK;
}

// ----------------------------------------
#include <esp_log.h>
#include <lib/support/CodeUtils.h>

#include <pir_sensor_task.h>

typedef struct {
    pir_sensor_config_t *config;
    bool is_initialized;
} pir_sensor_ctx_t;

static pir_sensor_ctx_t s_ctx;

static void pir_gpio_handler(void *arg)
{
    static bool occupancy = false;

    while (1) {
        bool new_occupancy = gpio_get_level(PIR_PIN);

        // we only need to notify application layer if occupancy changed
        if (occupancy != new_occupancy) {
            gpio_set_level(LED_PIN, new_occupancy);
            occupancy = new_occupancy;
            if (s_ctx.config->cb) {
                s_ctx.config->cb(s_ctx.config->endpoint_id, new_occupancy, s_ctx.config->user_data);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static void pir_sensor_gpio_init()
{
    // LED config
    gpio_config_t led_conf = {
        .pin_bit_mask = (1ULL << LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&led_conf);

    // Button config
    gpio_config_t btn_conf = {
        .pin_bit_mask = (1ULL << PIR_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&btn_conf);
}

esp_err_t pir_sensor_init(pir_sensor_config_t *config)
{
    pir_sensor_gpio_init();

    // Create the task
    xTaskCreatePinnedToCore(pir_gpio_handler, "pir_gpio_handler", 2048, NULL, 5, NULL, 1);

    s_ctx.config = config;
    s_ctx.is_initialized = true;
    return ESP_OK;
}