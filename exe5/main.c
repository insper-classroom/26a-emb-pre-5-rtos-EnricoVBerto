#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

const int BTN_RED_PIN = 28;
const int BTN_YELLOW_PIN = 21;

const int LED_RED_PIN = 5;
const int LED_YELLOW_PIN = 10;

#define BTN_RED_EVENT 1
#define BTN_YELLOW_EVENT 2

QueueHandle_t xQueueBtn;
SemaphoreHandle_t xSemaphoreLedR;
SemaphoreHandle_t xSemaphoreLedY;

void btn_callback(uint gpio, uint32_t events) {
    uint8_t btnEvent = 0;   

    if ((events & GPIO_IRQ_EDGE_FALL) == 0) {
        return;
    }

    if (gpio == BTN_RED_PIN) {
        btnEvent = BTN_RED_EVENT;
    } else if (gpio == BTN_YELLOW_PIN) {
        btnEvent = BTN_YELLOW_EVENT;
    }

    if (btnEvent != 0) {
        xQueueSendFromISR(xQueueBtn, &btnEvent, NULL);
    }
}

void redLedTask(void *p) {
    (void)p;
    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);

    int redBlinkEnabled = 0;
    int redLedState = 0;

    while (true) {
        if (xSemaphoreTake(xSemaphoreLedR, 0) == pdTRUE) {
            redBlinkEnabled = !redBlinkEnabled;
            if (!redBlinkEnabled) {
                redLedState = 0;
                gpio_put(LED_RED_PIN, 0);
            }
        }

        if (redBlinkEnabled) {
            redLedState = !redLedState;
            gpio_put(LED_RED_PIN, redLedState);
            vTaskDelay(pdMS_TO_TICKS(100));
        } else {
            gpio_put(LED_RED_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void yellowLedTask(void *p) {      
    (void)p;
    gpio_init(LED_YELLOW_PIN);
    gpio_set_dir(LED_YELLOW_PIN, GPIO_OUT);

    int yellowBlinkEnabled = 0;
    int yellowLedState = 0;

    while (true) {
        if (xSemaphoreTake(xSemaphoreLedY, 0) == pdTRUE) {
            yellowBlinkEnabled = !yellowBlinkEnabled;
            if (!yellowBlinkEnabled) {
                yellowLedState = 0;
                gpio_put(LED_YELLOW_PIN, 0);
            }
        }

        if (yellowBlinkEnabled) {
            yellowLedState = !yellowLedState;
            gpio_put(LED_YELLOW_PIN, yellowLedState);
            vTaskDelay(pdMS_TO_TICKS(100));
        } else {
            gpio_put(LED_YELLOW_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

void buttonTask(void *p) {
    (void)p;
    uint8_t receivedBtnEvent = 0;

    gpio_init(BTN_RED_PIN);
    gpio_set_dir(BTN_RED_PIN, GPIO_IN);
    gpio_pull_up(BTN_RED_PIN);

    gpio_init(BTN_YELLOW_PIN);
    gpio_set_dir(BTN_YELLOW_PIN, GPIO_IN);
    gpio_pull_up(BTN_YELLOW_PIN);

    gpio_set_irq_enabled_with_callback(BTN_RED_PIN, GPIO_IRQ_EDGE_FALL, true,
                                       &btn_callback);
    gpio_set_irq_enabled(BTN_YELLOW_PIN, GPIO_IRQ_EDGE_FALL, true);

    while (true) {
        if (xQueueReceive(xQueueBtn, &receivedBtnEvent, portMAX_DELAY) == pdTRUE) {
            if (receivedBtnEvent == BTN_RED_EVENT) {
                xSemaphoreGive(xSemaphoreLedR);
            } else if (receivedBtnEvent == BTN_YELLOW_EVENT) {
                xSemaphoreGive(xSemaphoreLedY);
            }
        }
    }
}

int main() {
    xQueueBtn = xQueueCreate(8, sizeof(uint8_t));
    xSemaphoreLedR = xSemaphoreCreateBinary();
    xSemaphoreLedY = xSemaphoreCreateBinary();

    xTaskCreate(redLedTask, "LED_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(yellowLedTask, "LED_Task 2", 256, NULL, 1, NULL);
    xTaskCreate(buttonTask, "BTN_Task", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while(1){}

    return 0;
}