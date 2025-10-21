/**
 * @file app_priv.h
 * @brief Private header for Matter smart plug application
 */
#pragma once

#include <esp_err.h>
#include <esp_matter.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include "esp_openthread_types.h"
#endif

/**
 * @brief GPIO pin for plug/relay/LED-Indicator control
 * 
 * Controls the main relay output:
 * - HIGH (1): Plug ON
 * - LOW (0): Plug OFF
 */
#define PLUG_GPIO               GPIO_NUM_22

/**
 * @brief GPIO pin for LED indicator
 * 
 * LED States:
 * - ON (steady): Device ready, plug OFF
 * - OFF (steady): Device ready, plug ON
 * - Blinking: Pairing mode active
 */
#define LED_GPIO                GPIO_NUM_19

/**
 * @brief GPIO pin for button input
 * 
 * Used for:
 * - Short press: Toggle plug ON/OFF
 * - Long press (5s): Start pairing or factory reset
 */
#define BUTTON_GPIO             GPIO_NUM_9


/**
 * @brief Opaque handle for app driver instance
 */
typedef void *app_driver_handle_t;

/**
 * @brief Initialize application driver
 * 
 * This function must be called after esp_matter::start().
 * It initializes:
 * - NVS flash storage
 * - GPIO pins (button, LED, plug relay)
 * - Boot reason check (handles auto-pairing after factory reset)
 * - Button monitoring task
 * 
 * @note This function will block briefly during initialization
 * 
 * @return void
 */
void app_driver_init(void);

/**
 * @brief Update driver based on Matter attribute changes
 * 
 * Called when Matter attributes are updated (e.g., from app/controller).
 * Updates physical GPIO state based on attribute values.
 * 
 * @param driver_handle Handle to driver instance (unused, can be NULL)
 * @param endpoint_id Endpoint ID of the attribute
 * @param cluster_id Cluster ID of the attribute
 * @param attribute_id Attribute ID being updated
 * @param val Pointer to new attribute value
 * 
 * @return ESP_OK on success, error code otherwise
 */

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#define ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG()                                           \
    {                                                                                   \
        .radio_mode = RADIO_MODE_NATIVE,                                                \
    }

#define ESP_OPENTHREAD_DEFAULT_HOST_CONFIG()                                            \
    {                                                                                   \
        .host_connection_mode = HOST_CONNECTION_MODE_NONE,                              \
    }

#define ESP_OPENTHREAD_DEFAULT_PORT_CONFIG()                                            \
    {                                                                                   \
        .storage_partition_name = "nvs", .netif_queue_size = 10, .task_queue_size = 10, \
    }
#endif
