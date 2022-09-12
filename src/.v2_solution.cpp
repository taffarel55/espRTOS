// v2_solution
// https://www.digikey.com.br/en/maker/projects/introduction-to-rtos-solution-to-part-2-freertos/b3f84c9c9455439ca2dcb8ccfce9dec5

#include <Arduino.h>
#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

static const int led_pin = LED_BUILTIN;

#define STACK_SIZE 1024
#define DELAY_MS 500
#define RATIO 3.141592

// Task blinking a LED
void toggleLED_slow(void *parameter) {
  while (1) {
    digitalWrite(led_pin, HIGH);
    vTaskDelay(DELAY_MS / portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(DELAY_MS / portTICK_PERIOD_MS);
  }
}

void toggleLED_fast(void *parameter) {
  while (1) {
    digitalWrite(led_pin, HIGH);
    vTaskDelay(DELAY_MS/RATIO / portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(DELAY_MS/RATIO / portTICK_PERIOD_MS);
  }
}

void setup() {
  pinMode(led_pin, OUTPUT);

  xTaskCreatePinnedToCore(  // Use xTaskCreate() in vanilla FreeRTOS
    toggleLED_fast,         // Task function
    "Toggle LED Fast",      // Task name
    STACK_SIZE,             // Stack size (bytes in ESP32, words in FreeRTOS)
    NULL,                   // Pointer parameter for the Task
    1,                      // Task priority (0 to configMAX_PRIORITES - 1)
    NULL,                   // Task handle
    app_cpu                 // Run on one core for demo purporses (ESP32 only)
  );

  xTaskCreatePinnedToCore(  // Use xTaskCreate() in vanilla FreeRTOS
    toggleLED_slow,         // Task function
    "Toggle LED Slow",      // Task name
    STACK_SIZE,             // Stack size (bytes in ESP32, words in FreeRTOS)
    NULL,                   // Pointer parameter for the Task
    1,                      // Task priority (0 to configMAX_PRIORITES - 1)
    NULL,                   // Task handle
    app_cpu                 // Run on one core for demo purporses (ESP32 only)
  );

  // If this was vanilla FreeRTOS, you'd want to call vTaskStartScheduler() 
  // in main after setting up your tasks
}

void loop() {
  
}