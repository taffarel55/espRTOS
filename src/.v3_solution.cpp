// .v3_solution
// monitor_speed = 9600

#include <Arduino.h>
#if CONFIG_FREERTOS_UNICORE
    static const BaseType_t app_cpu = 0;
#else
    static const BaseType_t app_cpu = 1;
#endif

// Led builtin
static const int led_pin = LED_BUILTIN;

// Task handles
static TaskHandle_t defineDelay = NULL;
static TaskHandle_t blinkingLed = NULL;

#define STACK_SIZE 1024
#define DELAY_MS 1000
#define START_DELAY_MS 1000

int input_delay_ms = 500;
static const uint8_t buf_len = 20;

// Task: Get the delay in milliseconds from user by serial
// void taskDefineDelay(void *parameter) {
//     while(1) {
//         int incomingByte = 0;
//         if (Serial.available() > 0) {
//             incomingByte = Serial.read();
            
//             // responde com o dado recebido:
//             Serial.print("I received: ");
//             Serial.println(incomingByte, DEC);  
            
//             // diz o novo delay
//             input_delay_ms = incomingByte * 10;
//             Serial.print("New delay: ");
//             Serial.println(input_delay_ms, DEC);
//         } 
//     }
// }

// Task: Get the delay in milliseconds from user by serial
void taskDefineDelay(void *parameter) {
    char c;
    char buf[buf_len];
    uint8_t idx = 0;

    // Clear whole buffer
    memset(buf, 0, buf_len);

    // Loop forever
    while (1) {

        // Read characters from serial
        if (Serial.available() > 0) {
        c = Serial.read();

        // Update delay variable and reset buffer if we get a newline character
        if (c == '\n') {
            input_delay_ms = atoi(buf);
            Serial.print("Updated LED delay to: ");
            Serial.println(input_delay_ms);
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

// Task: Blinks LED with the delay in milliseconds from user
void taskBlinkingLed(void *parameter) {
    while(1) {
        digitalWrite(led_pin, HIGH);
        vTaskDelay(input_delay_ms / portTICK_PERIOD_MS);
        digitalWrite(led_pin, LOW);
        vTaskDelay(input_delay_ms / portTICK_PERIOD_MS);
    }
}

void setup() {

    // Configure pin
    pinMode(led_pin, OUTPUT);

    // Configure serial and wait a second
    Serial.begin(9600);
    vTaskDelay(START_DELAY_MS / portTICK_PERIOD_MS);
    Serial.println("Multi-task LED Demo");
    Serial.println("Enter a number in milliseconds to change the LED delay.");

    xTaskCreatePinnedToCore(    // Use xTaskCreate() in vanilla FreeRTOS
        taskDefineDelay,        // Task function
        "Define delay",         // Task name
        STACK_SIZE,             // Stack size (bytes in ESP32, words in FreeRTOS)
        NULL,                   // Pointer parameter for the Task
        1,                      // Task priority (0 to configMAX_PRIORITES - 1)
        &defineDelay,           // Task handle
        app_cpu                 // Run on one core for demo purporses (ESP32 only)
    );

    xTaskCreatePinnedToCore(    // Use xTaskCreate() in vanilla FreeRTOS
        taskBlinkingLed,        // Task function
        "Blinks LED",           // Task name
        STACK_SIZE,             // Stack size (bytes in ESP32, words in FreeRTOS)
        NULL,                   // Pointer parameter for the Task
        2,                      // Task priority (0 to configMAX_PRIORITES - 1)
        &blinkingLed,           // Task handle
        app_cpu                 // Run on one core for demo purporses (ESP32 only)
    );

    // If this was vanilla FreeRTOS, you'd want to call vTaskStartScheduler() 
    // in main after setting up your tasks

    // Delete "setup and loop" task
    vTaskDelete(NULL);
}

void loop() {
    // Execution should never get here
}