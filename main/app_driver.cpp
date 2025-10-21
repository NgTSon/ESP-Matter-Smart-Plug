/**
 * @file app_driver.cpp
 * @brief Matter smart plug driver implementation
 * 
 * This driver handles:
 * - GPIO control (button, LED, relay)
 * - Button press detection (short/long press)
 * - Matter commissioning flow
 * - Factory reset
 * - NVS state persistence
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "esp_matter_console.h"
#include <esp_matter.h>
#include <app_priv.h>

using namespace chip::app::Clusters;
using namespace esp_matter;

// =====================================================================
// EXTERNAL REFERENCES
// =====================================================================

extern uint16_t plug_endpoint_id; /*Endpoint ID PLUG*/

// =====================================================================
// CONSTANTS & CONFIGURATION
// =====================================================================
static const char *TAG = "app_driver";

// NVS namespace for storing app state
static const char *NVS_NAMESPACE = "matter_app";
static const char *NVS_KEY_PENDING_PAIR = "pending_pair";

#define HOLD_TIME_MS            5000    // Long press threshold
#define DEBOUNCE_TIME_MS        50      // Button debounce delay

// =====================================================================
// GLOBAL VARIABLES
// =====================================================================

static bool plug_state = false;
static uint64_t button_press_time = 0;
static bool button_was_pressed = false;
static bool long_press_handled = false;


// =====================================================================
// GPIO INITIALIZATION
// =====================================================================

/**
 * @brief Configure all GPIO pins used by the application
 */
static void configure_gpios(void)
{
    // Configure button with pull-up (active LOW)
    gpio_config_t btn_cfg = {};
    btn_cfg.intr_type = GPIO_INTR_DISABLE;
    btn_cfg.mode = GPIO_MODE_INPUT;
    btn_cfg.pin_bit_mask = (1ULL << BUTTON_GPIO);
    btn_cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
    btn_cfg.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&btn_cfg);

    // Configure LED as output (ON = HIGH)
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO, 1); // Default ON (blue LED)

    // Configure plug relay as output (ON = HIGH)
    gpio_reset_pin(PLUG_GPIO);
    gpio_set_direction(PLUG_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(PLUG_GPIO, 0); // Default OFF (LED RED)
}


/*Update GPIO_Plug value*/
static esp_err_t app_driver_update_gpio_value(bool value)
{
    plug_state = value; // Value 

    esp_err_t err = gpio_set_level(PLUG_GPIO, value);

    gpio_set_level(LED_GPIO, !value); // LED inverse of plug
    
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set GPIO %d", PLUG_GPIO);
        return ESP_FAIL;
    }
    else{
      ESP_LOGI(TAG, "GPIO %d set to %d", PLUG_GPIO, value);
    }
    
    return err;
}


/**
 * @brief Handle Matter attribute updates from controller
 */
esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    esp_err_t err = ESP_OK;

    if (cluster_id == OnOff::Id) {
        if (attribute_id == OnOff::Attributes::OnOff::Id) {
            err = app_driver_update_gpio_value(val->val.b);
        }
    }
    return err;
}

// =====================================================================
// PLUG & LED CONTROL
// =====================================================================

/**
 * @brief Update plug GPIO and LED indicator
 * @param state true = plug ON, false = plug OFF
 * @brief Set plug state and sync with Matter attribute
 * @param state Desired plug state
 */
static void set_plug_state(bool state)
{
    plug_state = state;

    gpio_set_level(PLUG_GPIO, state);
    gpio_set_level(LED_GPIO, !state); // LED inverse of plug
    
    ESP_LOGI(TAG, "PLUG: %s, LED: %s", state ? "ON" : "OFF", !state ? "ON" : "OFF");

    // Update Matter value new OnOff attribute after toggle plug
    attribute_t * on_off_attr = attribute::get(plug_endpoint_id, OnOff::Id, OnOff::Attributes::OnOff::Id);

    if(on_off_attr){
        esp_matter_attr_val_t on_off_val = esp_matter_bool(NULL);
        attribute::get_val(on_off_attr, &on_off_val);
        on_off_val.val.b = state;
        attribute::update(plug_endpoint_id, OnOff::Id, OnOff::Attributes::OnOff::Id, &on_off_val);
    }
}

/**
 * @brief Toggle plug state
 */
static void toggle_plug(void)
{
    set_plug_state(!plug_state);
}


// =====================================================================
// MATTER COMISSIONING WINDOW FUNCTIONS
// =====================================================================

/**
 * @brief Get current fabric count
 * @return Number of fabrics paired
 */
static uint8_t get_fabric_count(void)
{
    return chip::Server::GetInstance().GetFabricTable().FabricCount();
}

/**
 * @brief Open Matter commissioning window
 */
static void start_matter_commissioning(void)
{
    ESP_LOGI(TAG, "Starting Matter commissioning...");
    
    using namespace chip;

    //timeout: 300 seconds
    constexpr uint16_t k_timeout_seconds = 300;
    constexpr System::Clock::Seconds16 kTimeoutSeconds(k_timeout_seconds);

    CommissioningWindowManager & commissionMgr = Server::GetInstance().GetCommissioningWindowManager();

    if (DeviceLayer::PlatformMgr().TryLockChipStack())
    {
        CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds);
        if (err != CHIP_NO_ERROR)
        {
            ESP_LOGE(TAG, "Failed to open commissioning window: %" CHIP_ERROR_FORMAT, err.Format());
        }
        else
        {
            ESP_LOGI(TAG, "Commissioning window opened for %u seconds", k_timeout_seconds);
        }

        DeviceLayer::PlatformMgr().UnlockChipStack();
    }
    else
{
        ESP_LOGE(TAG, "Failed to lock CHIP stack");
    }
    
    ESP_LOGI(TAG, "Matter commissioning started");
}


/**
 * @brief Close Matter commissioning window
 */
static void matter_stop_commissioning(void){
    if (chip::DeviceLayer::PlatformMgr().TryLockChipStack()){
        chip::Server::GetInstance().GetCommissioningWindowManager().CloseCommissioningWindow();
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
        ESP_LOGI(TAG, "Commissioning window closed");
    }
    else {
        ESP_LOGE(TAG, "Failed to lock CHIP stack to close commissioning window");
    }
}


// =====================================================================
// LED BLINK TASK (Pairing Indicator)
// =====================================================================

static TaskHandle_t blink_task_handle = NULL;

static void led_blink_task(void *arg)
{
    ESP_LOGI(TAG, "LED blink task started (waiting for pairing)");
    
    while (true) {
        // Blink LED 
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
        
        // Check if paired
        if (get_fabric_count() != 0) {
            ESP_LOGI(TAG, "Device paired! Stopping blink");
            gpio_set_level(LED_GPIO, 1);
            break;
        }
    }
    
    blink_task_handle = NULL;
    vTaskDelete(NULL);
}

/**
 * @brief Start LED blink task if not already running
 */
static void start_led_blink(void)
{
    if (blink_task_handle == NULL) {
        xTaskCreate(led_blink_task, "led_blink", 2048, NULL, 5, &blink_task_handle);
    }
}

/**
 * @brief Enter pairing mode (commissioning + LED blink)
 */
static void start_pairing_mode(void)
{
    ESP_LOGI(TAG, "Entering pairing mode");
    start_matter_commissioning();
    start_led_blink();
}

// =====================================================================
// NVS PERSISTENCE
// =====================================================================

/**
 * @brief Set pending pairing flag in NVS
 * @param pending true to set flag, false to clear
 */
static void set_pending_pairing_flag(bool value)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);

    if (err == ESP_OK) {
        nvs_set_u8(nvs_handle, NVS_KEY_PENDING_PAIR, value ? 1 : 0);
        nvs_commit(nvs_handle);
        nvs_close(nvs_handle);
        ESP_LOGI(TAG, "Saved pending pairing flag: %d", value);
    }
    else {
        ESP_LOGE(TAG, "Failed to open NVS for writing pending pairing flag");
    }
}

/**
 * @brief Get pending pairing flag from NVS
 * @return true if pairing is pending
 */
static bool get_pending_pairing_flag(void)
{   
    nvs_handle_t nvs_handle;
    uint8_t value = 0;

    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err == ESP_OK) {
        nvs_get_u8(nvs_handle, "pending_pair", &value);
        nvs_close(nvs_handle);
    }
    return (value == 1);
}
/**
 * @brief Clear pending pairing flag from NVS
 */
static void clear_pending_pairing_flag(void)
{
    set_pending_pairing_flag(false);
}


// =====================================================================
// BUTTON HANDLING
// =====================================================================

/**
 * @brief Handle short button press (< 5 seconds)
 */
static void handle_short_press(void)
{
    ESP_LOGI(TAG, "Short press - toggling plug");
    toggle_plug();
}

/**
 * @brief Handle long button press (>= 5 seconds)
 * 
 * Behavior:
 * - If not paired: Start pairing mode
 * - If paired: Factory reset + auto-pairing
 */
static void handle_long_press(void)
{
    uint8_t fabric_count = get_fabric_count();
    
    ESP_LOGI(TAG, "Long press detected (5s)");
    ESP_LOGI(TAG, "Fabric count: %d", fabric_count);
    
    if (fabric_count == 0) {
        // Not paired - start pairing
        ESP_LOGI(TAG, "Starting pairing mode");
        start_pairing_mode();
    } 
    else {
         // Already paired - factory reset
        ESP_LOGW(TAG, "Factory reset initiated");
        
        // Save flag to auto-pair after reset
        set_pending_pairing_flag(true);
        ESP_LOGW(TAG, "Performing factory reset...");
        
        // Visual feedback - fast blink
        for (int i = 0; i < 6; i++) {
            gpio_set_level(LED_GPIO, 1);
            vTaskDelay(pdMS_TO_TICKS(200));
            gpio_set_level(LED_GPIO, 0);
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        
        // Perform factory reset (will restart device)
        factory_reset();
    }
}

/**
 * @brief Button monitoring task
 * 
 * Continuously polls button state and handles:
 * - Debouncing
 * - Short press detection
 * - Long press detection (while holding)
 */
static void button_task(void *arg)
{
    ESP_LOGI(TAG, "Button task started");
    
    while (true) {
        bool current_level = gpio_get_level(BUTTON_GPIO);
        
        // Button pressed (LOW because pull-up)
        if (current_level == 0) {
            if (!button_was_pressed) {
                // First detection - debounce
                vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_TIME_MS));
            
                if (gpio_get_level(BUTTON_GPIO) == 0) {
                    // Confirmed press
                    button_was_pressed = true;
                    button_press_time = esp_timer_get_time() / 1000;
                    long_press_handled = false; 
                    ESP_LOGI(TAG, "Button pressed");
                }

            } else {
                // Still holding - check for long press
                uint64_t held_time = (esp_timer_get_time() / 1000) - button_press_time;
                if (held_time >= HOLD_TIME_MS && !long_press_handled) {
                    long_press_handled = true;
                    ESP_LOGI(TAG, "Long press triggered while still holding");
                    handle_long_press();
                }
            }
        }
        // Button released (HIGH)
        else if (current_level == 1 && button_was_pressed) {
            // Button released
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_TIME_MS));

            if (gpio_get_level(BUTTON_GPIO) == 1) {
                uint64_t press_duration = (esp_timer_get_time() / 1000) - button_press_time;
                button_was_pressed = false;

                ESP_LOGI(TAG, "Button released (held %llu ms)", press_duration);

                // Handle press if not already handled
                if (!long_press_handled) {
                    if (press_duration >= HOLD_TIME_MS) {
                        handle_long_press();  
                    } else {
                        handle_short_press();
                    }
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}


// =====================================================================
// BOOT HANDLING
// =====================================================================

/**
 * @brief Check boot reason and handle auto-pairing
 * 
 * Called on startup to:
 * - Check if auto-pairing is needed (after factory reset)
 * - Check current pairing status
 */

static void check_boot_reason(void)
{
    ESP_LOGI(TAG, "Checking boot reason...");
    
    if (get_pending_pairing_flag()) {
        // Auto-pairing after factory reset
        ESP_LOGI(TAG, "Auto-pairing after factory reset");
        //Clear NVS flag
        clear_pending_pairing_flag();
        // Start pairing mode
        start_pairing_mode();
    }

    else if (get_fabric_count() > 0) {
        // Already paired
        ESP_LOGI(TAG, "Device already paired (Fabric count: %d)", get_fabric_count());
    }
    else {
        ESP_LOGE(TAG, "Device not paired, waiting for button action");
    }
}

// =====================================================================
// INITIALIZATION
// =====================================================================

void app_driver_init(void)
{
    matter_stop_commissioning();
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Configure GPIOs
    configure_gpios();
    
    // Check boot reason and process
    check_boot_reason();
    
    // Start button monitoring task
    xTaskCreate(button_task, "button_task", 4096, NULL, 10, NULL);
    
    ESP_LOGI(TAG, "Matter Button Control initialized\n");
}
