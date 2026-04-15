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

QueueHandle_t redButtonQueue;
QueueHandle_t greenButtonQueue;

void redLedTask(void *p) {
    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);

    int redDelay = 0;
    while (true) {
        if (xQueueReceive(redButtonQueue, &redDelay, 0)) {
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

    int redDelay = 0;
    while (true) {
        if (!gpio_get(BTN_RED_PIN)) {

            while (!gpio_get(BTN_RED_PIN)) {
                vTaskDelay(pdMS_TO_TICKS(1));
            }

            if (redDelay < 1000) {
                redDelay += 100;
            } else {
                redDelay = 100;
            }
            printf("delay btn %d \n", redDelay);
            xQueueSend(redButtonQueue, &redDelay, 0);
        }
    }
}

void greenLedTask(void *p) {
    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);

    int greenDelay = 0;
    while (true) {
        if (xQueueReceive(greenButtonQueue, &greenDelay, 0)) {
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

    int greenDelay = 0;
    while (true) {
        if (!gpio_get(BTN_GREEN_PIN)) {

            while (!gpio_get(BTN_GREEN_PIN)) {
                vTaskDelay(pdMS_TO_TICKS(1));
            }

            if (greenDelay < 1000) {
                greenDelay += 100;
            } else {
                greenDelay = 100;
            }
            printf("delay btn2 %d \n", greenDelay);
            xQueueSend(greenButtonQueue, &greenDelay, 0);
        }
    }
}


int main() {
    stdio_init_all();
    printf("Start RTOS \n");

    redButtonQueue = xQueueCreate(32, sizeof(int));
    greenButtonQueue = xQueueCreate(32, sizeof(int));

    xTaskCreate(redLedTask, "LED_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(redBtnTask, "BTN_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(greenLedTask, "LED_Task 2", 256, NULL, 1, NULL);
    xTaskCreate(greenBtnTask, "BTN_Task 2", 256, NULL, 1, NULL);
    
    vTaskStartScheduler();

    while (true)
        ;
}
