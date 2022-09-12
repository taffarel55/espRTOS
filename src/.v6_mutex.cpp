
#include <Arduino.h>
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Globals
static int shared_var = 0;
static SemaphoreHandle_t mutex;

//*****************************************************************************
// Tasks

// Increment shared variable (the wrong way)
void taskIncrementVariable(void *parameters) {
  int local_var;

  while (1) {
    // Take mutex prior to critical section
    if(xSemaphoreTake(mutex,0) == pdTRUE) {
      local_var = shared_var;
      local_var++;
      vTaskDelay(random(100,500) / portTICK_PERIOD_MS);
      shared_var = local_var;

      // Give mutex after critical section
      xSemaphoreGive(mutex);

      Serial.println(shared_var);
    } else {
        // Do something else
    }
  }
}



//*****************************************************************************
// Main (runs as its own task with priority 1 on core 1)

void setup() {

  // Configure serial
  randomSeed(analogRead(0));

  // Configure Serial
  Serial.begin(115200);

  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS Mutex---");

  // Create mutex before starting tasks
  mutex = xSemaphoreCreateMutex();

  // Start tasks
  xTaskCreatePinnedToCore(taskIncrementVariable,
                          "Increment task 1",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);

  xTaskCreatePinnedToCore(taskIncrementVariable,
                          "Increment task 1",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);

  // Delete "setup and loop" task
  vTaskDelete(NULL);
}

void loop() {
  // Execution should never get here
}