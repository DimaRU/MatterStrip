/*
    On-board button driver
*/

#include <esp_log.h>
#include <stdlib.h>
#include <string.h>

#include <common_macros.h>
#include <iot_button.h>
#include <button_gpio.h>
#include <app_priv.h>

static const char *TAG = "button_driver";

static const button_config_t button_config = {
    .long_press_time = CONFIG_BUTTON_LONG_PRESS_TIME_MS,
    .short_press_time = CONFIG_BUTTON_SHORT_PRESS_TIME_MS,
};

const button_gpio_config_t btn_gpio_cfg = {
    .gpio_num = CONFIG_BUTTON_GPIO,
    .active_level = 0,
};

static void button_single_click_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "Toggle button press");
    void (*button_toggle_callback)() = (void (*)())data;
    button_toggle_callback();
}

static void button_long_press_up_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "Button long press");
    void (*button_reset_callback)() = (void (*)())data;
    button_reset_callback();
}

void app_driver_button_init(void (*button_toggle_callback)(), void (*button_reset_callback)()) {
	button_handle_t button_handle;
    esp_err_t err = iot_button_new_gpio_device(&button_config, &btn_gpio_cfg, &button_handle);
    ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG, "Failed to create button handle"));
	err |= iot_button_register_cb(button_handle, BUTTON_SINGLE_CLICK, NULL, button_single_click_cb, (void*)button_toggle_callback);
    err |= iot_button_register_cb(button_handle, BUTTON_LONG_PRESS_UP, NULL, button_long_press_up_cb, (void*)button_reset_callback);
    ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG, "Failed to register callback"));
}
