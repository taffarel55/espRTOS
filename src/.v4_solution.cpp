// v4_solution

#include <Arduino.h>
#if CONFIG_FREERTOS_UNICORE
    static const BaseType_t app_cpu = 0;
#else
    static const BaseType_t app_cpu = 1;
#endif

// Task handles
static TaskHandle_t saveMessageOnHeap = NULL;
static TaskHandle_t printsMessageFromHeap = NULL;

#define STACK_SIZE 1024
#define DELAY_MS 1000
#define START_DELAY_MS 1000
#define TAM 100
#define BYTES_ALLOCATED 1024

// Settings
static const uint8_t buf_len = 255;

// Globals
static char *msg_ptr = NULL;
static volatile uint8_t bufferSaved = 0;

void taskSaveMessageOnHeap(void *parameter) {
    char c;  
    uint8_t idx = 0;
    char buf[buf_len];

    // Clear whole buffer
    memset(buf, 0, buf_len);

    // Loop forever
    while (1) {

        // Read characters from serial
        if (Serial.available() > 0) {

            c = Serial.read();

            // Update delay variable and reset buffer if we get a newline character
            if (c == '\n') {
                // Prints buffer
                // The last character in the string is '\n', so we need to replace
                // it with '\0' to make it null-terminated
                buf[idx - 1] = '\0';
                Serial.println(buf);

                // Try to allocate memory and copy over message. If message buffer is
                // still in use, ignore the entire message.
                if (!bufferSaved) {
                    msg_ptr = (char *)pvPortMalloc(idx * sizeof(char));

                    // If malloc returns 0 (out of memory), throw an error and reset
                    configASSERT(msg_ptr);

                    // Copy message
                    memcpy(msg_ptr, buf, idx);

                    // Notify other task that message is ready
                    bufferSaved = 1;
                }

                // Clears the buffer
                memset(buf, 0, buf_len);
                idx = 0;
            } else {
                Serial.print(c);
                // Only append if index is not over message limit
                if (idx < buf_len - 1) {
                    buf[idx] = c;
                    idx++;
                }
            }
        }
    }
}

void taskPrintsMessageFromHeap(void *parameter) {
    while (1) {
        if (bufferSaved) {
            Serial.println(msg_ptr);

            // Give amount of free heap memory 
            Serial.print("Free heap (bytes): ");
            Serial.println(xPortGetFreeHeapSize());

            // Free buffer, set pointer to null, and clear flag
            vPortFree(msg_ptr);
            msg_ptr = NULL;
            bufferSaved = 0;
        }
    }
}

void setup() {

    // Configure Serial
    Serial.begin(115200);

    // Wait a moment to start (so we don't miss Serial output)
    vTaskDelay(START_DELAY_MS / portTICK_PERIOD_MS);

    Serial.println();
    Serial.println("---FreeRTOS Heap Demo---");
    Serial.println("Enter a string");

    xTaskCreatePinnedToCore(    // Use xTaskCreate() in vanilla FreeRTOS
        taskSaveMessageOnHeap,  // Task function
        "Gets message",         // Task name
        STACK_SIZE,             // Stack size (bytes in ESP32, words in FreeRTOS)
        NULL,                   // Pointer parameter for the Task
        1,                      // Task priority (0 to configMAX_PRIORITES - 1)
        &saveMessageOnHeap,     // Task handle
        app_cpu                 // Run on one core for demo purporses (ESP32 only)
    );

    xTaskCreatePinnedToCore(       // Use xTaskCreate() in vanilla FreeRTOS
        taskPrintsMessageFromHeap, // Task function
        "Prints Message",          // Task name
        STACK_SIZE,                // Stack size (bytes in ESP32, words in FreeRTOS)
        NULL,                      // Pointer parameter for the Task
        1,                         // Task priority (0 to configMAX_PRIORITES - 1)
        &printsMessageFromHeap,    // Task handle
        app_cpu                    // Run on one core for demo purporses (ESP32 only)
    );

    // If this was vanilla FreeRTOS, you'd want to call vTaskStartScheduler() 
    // in main after setting up your tasks

    vTaskDelete(NULL);
}

void loop() {
    // Execution should never get here
}