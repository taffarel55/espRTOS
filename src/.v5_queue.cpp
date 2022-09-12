// v5_queue

#include <Arduino.h>
#if CONFIG_FREERTOS_UNICORE
    static const BaseType_t app_cpu = 0;
#else
    static const BaseType_t app_cpu = 1;
#endif

// Task handles
static TaskHandle_t printsMessages = NULL;

#define STACK_SIZE 1024
#define DELAY_MS 1000
#define DELAY_FILLING_MS 500
#define START_DELAY_MS 1000
#define TAM 100
#define BYTES_ALLOCATED 1024

// Settings
static const uint8_t msg_queue_len = 5;

// Globals
static QueueHandle_t msg_queue;

// Tasks

void taskPrintsMessages(void *parameter) {
    int item;
    while (1) {
        // See if there's a message in the queue (do not block)
        if (xQueueReceive(msg_queue, (void *)&item, 0) == pdTRUE) {
            Serial.println(item);
        }

        // Wait before trying again
        vTaskDelay(DELAY_MS / portTICK_PERIOD_MS);
    }
}

void setup() {

    // Configure Serial
    Serial.begin(115200);

    // Wait a moment to start (so we don't miss Serial output)
    vTaskDelay(START_DELAY_MS / portTICK_PERIOD_MS);

    Serial.println();
    Serial.println("---FreeRTOS Queue Demo---");

    msg_queue = xQueueCreate(msg_queue_len, sizeof(int));

    xTaskCreatePinnedToCore(       // Use xTaskCreate() in vanilla FreeRTOS
        taskPrintsMessages,        // Task function
        "Prints Message",          // Task name
        STACK_SIZE,                // Stack size (bytes in ESP32, words in FreeRTOS)
        NULL,                      // Pointer parameter for the Task
        1,                         // Task priority (0 to configMAX_PRIORITES - 1)
        &printsMessages,           // Task handle
        app_cpu                    // Run on one core for demo purporses (ESP32 only)
    );

    // If this was vanilla FreeRTOS, you'd want to call vTaskStartScheduler() 
    // in main after setting up your tasks
}

void loop() {
    static int num = 0;

    // Try to add item to queue for 10 ticks, fail if the queue is full
    if(xQueueSend(msg_queue, (void *)&num, 10) != pdTRUE) {
        Serial.println("Queue full");
    }
    
    num++;

    // Wait before trying again
    vTaskDelay(DELAY_FILLING_MS / portTICK_PERIOD_MS);
}