#include "led_service.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

const char *TAG = "LED_SERVICE";

typedef struct {
    led_state_t state;
    uint32_t period_ms;
    TickType_t last_toggle;
    uint32_t level;
} led_runtime_t;

static const gpio_num_t LED_PINS[] = {
    GPIO_NUM_4,
    GPIO_NUM_5,
    GPIO_NUM_6
};

static TaskHandle_t led_task_handle;
static QueueHandle_t led_queue;

static led_runtime_t led_state[LED_ID_SIZE];

static esp_err_t led_command(const led_command_t* command) {
    if(!command) return ESP_ERR_INVALID_ARG;

    led_state[command->id].state = command->state;
    led_state[command->id].period_ms = command->period_ms;
    led_state[command->id].last_toggle = xTaskGetTickCount();

    if (command->state == LED_STATE_BLINK) {
        led_state[command->id].level = 0;
        return gpio_set_level(LED_PINS[command->id], 0);
    } 
    else {
        led_state[command->id].level = command->state;
        return gpio_set_level(LED_PINS[command->id], command->state);
    }
}

static void led_service_task(void *arg) {
    const TickType_t tick_interval = pdMS_TO_TICKS(50);
    for(;;) {
        led_command_t command;

        while(xQueueReceive(led_queue, &command, 0) == pdTRUE) {
            esp_err_t err = led_command(&command);
            if(err == ESP_OK) {
                ESP_LOGI(TAG, "Set LED %d to %d", command.id, command.state);
            }
            else {
                ESP_LOGE(TAG, "Failed to set LED level");
            }
        }

        TickType_t now = xTaskGetTickCount();
        for(size_t i = 0; i < LED_ID_SIZE; i++) {
            if(led_state[i].state == LED_STATE_BLINK) {
                if(now - led_state[i].last_toggle >= pdMS_TO_TICKS(led_state[i].period_ms)) {
                    led_state[i].level = !led_state[i].level;
                    gpio_set_level(LED_PINS[i], led_state[i].level);
                    led_state[i].last_toggle = now;
                }
            }
        }
        vTaskDelay(tick_interval);
    }
}

esp_err_t led_service_init(void) {
    /*Configuration of the LED GPIO Pins*/
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    uint64_t pin_mask = 0;
    for (int i = 0; i < LED_ID_SIZE; i++) {
        pin_mask |= 1ULL << LED_PINS[i];
    }

    io_conf.pin_bit_mask = pin_mask;
    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "GPIO config failed: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "LED GPIO Configuration Initialized");

    /*Initialize LED states*/
    for (size_t i = 0; i < LED_ID_SIZE; i++) {
        led_state[i].state = LED_STATE_LOW;
        led_state[i].level = 0;
        led_state[i].period_ms = 0;
        led_state[i].last_toggle = 0;
    }

    /*LED Service Queue and Task initialization*/
    led_queue = xQueueCreate(10, sizeof(led_command_t));
    BaseType_t ok = xTaskCreate(led_service_task, "LED Task", 4096, NULL, 5, &led_task_handle);
    
    return ok == pdPASS ? ESP_OK : ESP_ERR_NO_MEM;
}

esp_err_t led_service_set_led(led_id_t id, led_state_t state, uint32_t period_ms) {
    if(id >= LED_ID_SIZE || state >= LED_STATE_SIZE) return ESP_ERR_INVALID_ARG;
    led_command_t command = {
        .id = id,
        .state = state,
        .period_ms = period_ms
    };
    BaseType_t ok = xQueueSend(led_queue, &command, portMAX_DELAY);
    return ok == pdPASS ? ESP_OK : ESP_ERR_TIMEOUT;
}
