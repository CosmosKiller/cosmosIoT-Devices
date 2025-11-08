/**
 * @file encoder_task.cpp
 * @author Marcel Nahir Samur (mnsamur2014@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-10-05
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <encoder_task.h>
#include <esp_log.h>

static const char *TAG = "encoder_task";

esp_err_t encoder_init(encoder_config_t *encoder)
{
    // PCNT unit configuration
    pcnt_unit_config_t unit_config = {
        .low_limit = PCNT_LOW_LIMIT,
        .high_limit = PCNT_HIGH_LIMIT,
    };
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &encoder->pcnt_unit));

    // PCNT channel configuration
    ESP_LOGI(TAG, "install pcnt channels");
    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num = encoder->pin_a,
        .level_gpio_num = encoder->pin_b,
    };
    pcnt_channel_handle_t pcnt_chan_a = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(encoder->pcnt_unit, &chan_a_config, &pcnt_chan_a));

    pcnt_chan_config_t chan_b_config = {
        .edge_gpio_num = encoder->pin_b,
        .level_gpio_num = encoder->pin_a,
    };
    pcnt_channel_handle_t pcnt_chan_b = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(encoder->pcnt_unit, &chan_b_config, &pcnt_chan_b));

    // Set edge and level actions
    ESP_LOGI(TAG, "set edge and level actions for pcnt channels");
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    // Enable hardware filter
    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = ENCODER_FILTER_NS,
    };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(encoder->pcnt_unit, &filter_config));

    // Start PCNT unit
    ESP_ERROR_CHECK(pcnt_unit_enable(encoder->pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(encoder->pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_start(encoder->pcnt_unit));

    // Initialize other encoder parameters
    encoder->counter = 0;

    return ESP_OK;
}