#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>

const int BTN_RED_PIN = 28;
const int BTN_GREEN_PIN = 26;

const int LED_RED_PIN = 4;
const int LED_GREEN_PIN = 6;

QueueHandle_t redDelayQueue;
SemaphoreHandle_t redBtnSemaphore;
QueueHandle_t greenDelayQueue;
SemaphoreHandle_t greenBtnSemaphore;

void btn_callback(uint gpio, uint32_t events) {
    if (gpio == BTN_RED_PIN && events == 0x4) { // fall edge
        xSemaphoreGiveFromISR(redBtnSemaphore, 0);
    }
    if (gpio == BTN_GREEN_PIN && events == 0x4) { // fall edge
        xSemaphoreGiveFromISR(greenBtnSemaphore, 0);
    }
}

void redLedTask(void *p) {
    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);

    int redDelay = 0;

    while (true) {
        if (xQueueReceive(redDelayQueue, &redDelay, 0)) {
            printf("%d\n", redDelay);
        }

        if (redDelay > 0) {
            gpio_put(LED_RED_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(redDelay));
            gpio_put(LED_RED_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(redDelay));
        }
    }
}

void redBtnTask(void *p) {
    gpio_init(BTN_RED_PIN);
    gpio_set_dir(BTN_RED_PIN, GPIO_IN);
    gpio_pull_up(BTN_RED_PIN);
    gpio_set_irq_enabled_with_callback(BTN_RED_PIN, GPIO_IRQ_EDGE_FALL, true,
                                       &btn_callback);

    int redDelay = 0;
    while (true) {
        if (xSemaphoreTake(redBtnSemaphore, pdMS_TO_TICKS(500)) == pdTRUE) {
            if (redDelay < 1000) {
                redDelay += 100;
            } else {
                redDelay = 100;
            }
            printf("delay btn %d \n", redDelay);
            xQueueSend(redDelayQueue, &redDelay, 0);
        }
    }
}

void greenLedTask(void *p) {
    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);

    int greenDelay = 0;

    while (true) {
        if (xQueueReceive(greenDelayQueue, &greenDelay, 0)) {
            printf("%d\n", greenDelay);
        }

        if (greenDelay > 0) {
            gpio_put(LED_GREEN_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(greenDelay));
            gpio_put(LED_GREEN_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(greenDelay));
        }
    }
}

void greenBtnTask(void *p) {
    gpio_init(BTN_GREEN_PIN);
    gpio_set_dir(BTN_GREEN_PIN, GPIO_IN);
    gpio_pull_up(BTN_GREEN_PIN);
    gpio_set_irq_enabled_with_callback(BTN_GREEN_PIN, GPIO_IRQ_EDGE_FALL, true,
                                       &btn_callback);

    int greenDelay = 0;
    while (true) {
        if (xSemaphoreTake(greenBtnSemaphore, pdMS_TO_TICKS(500)) == pdTRUE) {
            if (greenDelay < 1000) {
                greenDelay += 100;
            } else {
                greenDelay = 100;
            }
            printf("delay btn %d \n", greenDelay);
            xQueueSend(greenDelayQueue, &greenDelay, 0);
        }
    }
}

int main() {
    stdio_init_all();
    printf("Start RTOS \n");

    redDelayQueue = xQueueCreate(32, sizeof(int));
    greenDelayQueue = xQueueCreate(32, sizeof(int));
    redBtnSemaphore = xSemaphoreCreateBinary();
    greenBtnSemaphore = xSemaphoreCreateBinary();

    xTaskCreate(redLedTask, "LED_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(redBtnTask, "BTN_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(greenLedTask, "LED_Task 2", 256, NULL, 1, NULL);
    xTaskCreate(greenBtnTask, "BTN_Task 2", 256, NULL, 1, NULL);


    vTaskStartScheduler();

    while (true)
        ;
}
