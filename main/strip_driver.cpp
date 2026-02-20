/*
    App driver
*/

#include <esp_log.h>
#include <stdlib.h>
#include <string.h>

#include <esp_matter.h>
#include <common_macros.h>
#include <app_priv.h>

#include "driver/gpio.h"

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

static const char *TAG = "strip_driver";

static uint16_t powerOnMask;
static uint16_t endpointIdMap[CONFIG_OUTLET_COUNT];
static const uint8_t pinMap[CONFIG_OUTLET_COUNT] = {
#ifdef CONFIG_GPIO_OUTLET_1
    CONFIG_GPIO_OUTLET_1,
#endif
#ifdef CONFIG_GPIO_OUTLET_2
    CONFIG_GPIO_OUTLET_2,
#endif
#ifdef CONFIG_GPIO_OUTLET_3
    CONFIG_GPIO_OUTLET_3,
#endif
#ifdef CONFIG_GPIO_OUTLET_4
    CONFIG_GPIO_OUTLET_4,
#endif
#ifdef CONFIG_GPIO_OUTLET_5
    CONFIG_GPIO_OUTLET_5,
#endif
#ifdef CONFIG_GPIO_OUTLET_6
    CONFIG_GPIO_OUTLET_6,
#endif
};

static void app_driver_set_outlet_state(int index, bool value)
{
    gpio_num_t pin = (gpio_num_t)pinMap[index];
    gpio_set_level(pin, value);
    ESP_LOGI(TAG, "GPIO pin : %d(%d) set to %d", pin, index, value);

    auto powerOnMaskPrev = powerOnMask;
    uint16_t mask = 1 << index;
    if (value) {
        powerOnMask |= mask;
    } else {
        powerOnMask &= ~mask;
    }
#if CONFIG_POWER_LED
    if ((powerOnMaskPrev == 0) && (powerOnMask != 0)) {
        gpio_set_level((gpio_num_t)CONFIG_POWER_LED_GPIO, 1);
    } else if ((powerOnMaskPrev != 0) && (powerOnMask == 0)) {
        gpio_set_level((gpio_num_t)CONFIG_POWER_LED_GPIO, 0);
    }
#endif
}

void app_driver_attribute_update(uint16_t endpoint_id,
                                 uint32_t cluster_id,
                                 uint32_t attribute_id,
                                 esp_matter_attr_val_t *val)
{
    if ((cluster_id != OnOff::Id) || (attribute_id != OnOff::Attributes::OnOff::Id)) {
        return;
    }
    for(int index = 0; index < CONFIG_OUTLET_COUNT; index++) {
        if (endpointIdMap[index] == endpoint_id) {
            app_driver_set_outlet_state(index, val->val.b);
            break;
        }
    }
}


void app_driver_create_endpoints(esp_matter::node_t *node) {
    for(int index = 0; index < CONFIG_OUTLET_COUNT; index++) {
        on_off_plug_in_unit::config_t plugin_unit_config;
        plugin_unit_config.on_off.on_off = false;
        endpoint_t *endpoint = on_off_plug_in_unit::create(node, &plugin_unit_config, ENDPOINT_FLAG_NONE, (void *)&pinMap[index]);
        ABORT_APP_ON_FAILURE(endpoint != nullptr, ESP_LOGE(TAG, "Matter endpoint creation failed"));
        endpointIdMap[index] = endpoint::get_id(endpoint);
    }
}

/* Starting driver with default values */
void app_driver_restore_matter_state() {
    esp_matter_attr_val_t val = esp_matter_invalid(NULL);
    attribute_t *attribute;
    for(int index = 0; index < CONFIG_OUTLET_COUNT; index++) {
        attribute = attribute::get(endpointIdMap[index], OnOff::Id, OnOff::Attributes::OnOff::Id);
        attribute::get_val(attribute, &val);
        app_driver_set_outlet_state(index, val.val.b);
    }
}

// Button toggle callback
void button_toggle_cb()
{
    uint32_t cluster_id = OnOff::Id;
    uint32_t attribute_id = OnOff::Attributes::OnOff::Id;
    esp_matter_attr_val_t val = esp_matter_invalid(NULL);

    bool onFlag = powerOnMask == 0;
    for(int index = 0; index < CONFIG_OUTLET_COUNT; index++) {
        auto endpoint_id = endpointIdMap[index];
        attribute_t *attribute = attribute::get(endpoint_id, cluster_id, attribute_id);
        attribute::get_val(attribute, &val);
        if (onFlag) {
            // All on
            val.val.b = true;
            attribute::update(endpoint_id, cluster_id, attribute_id, &val);
        } else {
            // All off
            if (val.val.b) {
                val.val.b = false;
                attribute::update(endpoint_id, cluster_id, attribute_id, &val);
            }
        }
    }
}

// Print hardware config
static void printHardwareConfig() {
    for(int index = 0; index < CONFIG_OUTLET_COUNT; index++) {
        ESP_LOGI(TAG, "Outlet %i pin: %i", index, pinMap[index]);
    }

    ESP_LOGI(TAG, "Button pin: %i", CONFIG_BUTTON_GPIO);
#if CONFIG_INDICATOR_LED_INVERT
    ESP_LOGI(TAG, "Indicator led pin: %i (inverted)", CONFIG_INDICATOR_LED_GPIO);
#else
    ESP_LOGI(TAG, "Indicator led pin: %i", CONFIG_INDICATOR_LED_GPIO);
#endif
}

void app_driver_init() {
    printHardwareConfig();
    for(int index = 0; index < CONFIG_OUTLET_COUNT; index++) {
        auto pin = gpio_num_t(pinMap[index]);
        gpio_reset_pin(pin);
        gpio_set_direction(pin, GPIO_MODE_OUTPUT);
        gpio_set_level(pin, 0);
    }
#if CONFIG_POWER_LED
    gpio_reset_pin((gpio_num_t)CONFIG_POWER_LED_GPIO);
    gpio_set_direction((gpio_num_t)CONFIG_POWER_LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)CONFIG_POWER_LED_GPIO, 0);
#endif    
    powerOnMask = 0;
}
