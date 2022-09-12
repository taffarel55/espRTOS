// v4_memory_management

#include <Arduino.h>
#if CONFIG_FREERTOS_UNICORE
    static const BaseType_t app_cpu = 0;
#else
    static const BaseType_t app_cpu = 1;
#endif

// Task handles
static TaskHandle_t memoryLeak = NULL;

#define STACK_SIZE 2024
#define DELAY_MS 1000
#define START_DELAY_MS 1000
#define TAM 100
#define BYTES_ALLOCATED 1024

void taskMemoryLeak(void *parameter) {
    while(1) {
        int a = 1;
        int b[TAM];

        // Do something with array so it's not optimized out by the compiler
        for(int i=0; i<TAM; i++) {
            b[i] = a + 1;
        }
        Serial.println(b[0]); // To prevent cpp warnings

        // Print out remaining stack memory (words)
        Serial.print("High water mark (words): ");
        Serial.println(uxTaskGetStackHighWaterMark(NULL));

        // Print out number of free heap memory bytes before malloc
        Serial.print("Heap before malloc (bytes): ");
        Serial.println(xPortGetFreeHeapSize());

        int *ptr = (int*) pvPortMalloc(BYTES_ALLOCATED * sizeof(int));

        // One way to prevent heap overflow is to check the mallot output
        if (ptr == NULL) {
            Serial.println("Not enough heap");
        } else {
            // Do something with memory so it's not optimized out by the compiler
            for(int i = 0; i < BYTES_ALLOCATED; i++) {
                ptr[i] = 3;
            }
        }

        Serial.print("Heap after malloc (bytes): ");
        Serial.println(xPortGetFreeHeapSize());

        // Free up our allocated memory
        vPortFree(ptr);

        vTaskDelay(DELAY_MS / portTICK_PERIOD_MS);
    }
}

void setup() {

    Serial.begin(115200);
    vTaskDelay(START_DELAY_MS / portTICK_PERIOD_MS);

    xTaskCreatePinnedToCore(    // Use xTaskCreate() in vanilla FreeRTOS
        taskMemoryLeak,         // Task function
        "Test memory leak",     // Task name
        STACK_SIZE,             // Stack size (bytes in ESP32, words in FreeRTOS)
        NULL,                   // Pointer parameter for the Task
        1,                      // Task priority (0 to configMAX_PRIORITES - 1)
        &memoryLeak,            // Task handle
        app_cpu                 // Run on one core for demo purporses (ESP32 only)
    );

    // If this was vanilla FreeRTOS, you'd want to call vTaskStartScheduler() 
    // in main after setting up your tasks

    vTaskDelete(NULL);
}

void loop() {
    // Try to send backspace and + serial 
}