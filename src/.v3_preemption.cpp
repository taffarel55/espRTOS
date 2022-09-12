// v3_preemption
// monitor_speed = 300

#include <Arduino.h>
#if CONFIG_FREERTOS_UNICORE
    static const BaseType_t app_cpu = 0;
#else
    static const BaseType_t app_cpu = 1;
#endif

// Some string to print
const char msg[] = "Estou aprendendo RTOS";

// Led builtin
static const int led_pin = LED_BUILTIN;

// Task handles
static TaskHandle_t printMessage = NULL;
static TaskHandle_t printAsterisks = NULL;

#define STACK_SIZE 1024
#define DELAY_MS 1000
#define START_DELAY_MS 1000
#define SUSPEND_DELAY_MS 2000

// Task: print to Serial Terminal with lower priority
void taskPrintMessage(void *parameter) {
    
    int msg_len = strlen(msg);

    while (1) {
        digitalWrite(led_pin, HIGH);
        Serial.println();
        for(int i=0; i<msg_len;i++) {
            Serial.print(msg[i]);
        }
        Serial.println();
        digitalWrite(led_pin, LOW);
        vTaskDelay(DELAY_MS/ portTICK_PERIOD_MS);
    }

}

// Task: print to Serial Terminal with lower priority
void taskPrintAsterisks(void *parameter) {

    while (1) {
        Serial.print("*");
        vTaskDelay(DELAY_MS / portTICK_PERIOD_MS);
    }

}

void setup() {

    pinMode(led_pin, OUTPUT);
    Serial.begin(300);
    vTaskDelay(START_DELAY_MS / portTICK_PERIOD_MS);

    Serial.println("");
    Serial.println("--- FreeRTOS Task Demo ---");
    Serial.print("Setup and loop task running on core ");
    Serial.print(xPortGetCoreID());
    Serial.print(" with priority ");
    Serial.println(uxTaskPriorityGet(NULL));

    xTaskCreatePinnedToCore(    // Use xTaskCreate() in vanilla FreeRTOS
        taskPrintMessage,       // Task function
        "Print message",        // Task name
        STACK_SIZE,             // Stack size (bytes in ESP32, words in FreeRTOS)
        NULL,                   // Pointer parameter for the Task
        1,                      // Task priority (0 to configMAX_PRIORITES - 1)
        &printMessage,          // Task handle
        app_cpu                 // Run on one core for demo purporses (ESP32 only)
    );

    xTaskCreatePinnedToCore(    // Use xTaskCreate() in vanilla FreeRTOS
        taskPrintAsterisks,     // Task function
        "Print Asterisks",      // Task name
        STACK_SIZE,             // Stack size (bytes in ESP32, words in FreeRTOS)
        NULL,                   // Pointer parameter for the Task
        2,                      // Task priority (0 to configMAX_PRIORITES - 1)
        &printAsterisks,        // Task handle
        app_cpu                 // Run on one core for demo purporses (ESP32 only)
    );

  // If this was vanilla FreeRTOS, you'd want to call vTaskStartScheduler() 
  // in main after setting up your tasks
}

void loop() {
    // Suspend the higher priority task for some intervals
    for(int i=0; i<3;i++) {
        Serial.println("");
        Serial.print("Suspend task ");
        Serial.println((long) &printAsterisks);
        vTaskSuspend(printAsterisks);
        vTaskDelay(SUSPEND_DELAY_MS / portTICK_PERIOD_MS);
        vTaskResume(printAsterisks);
        vTaskDelay(SUSPEND_DELAY_MS / portTICK_PERIOD_MS);
    }

    // Delete the lower
    if (printMessage != NULL) {
        Serial.print("Deleting task ");
        Serial.println((long) &printMessage);
        vTaskDelete(printMessage);
        printMessage = NULL;
    }
}