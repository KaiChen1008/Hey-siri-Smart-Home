#include <Arduino_FreeRTOS.h>
#include <Stepper.h>
 
#define fanPin 3
#define irPin A3
#define light 4
#define STEPS 2048
 
Stepper stepper(STEPS, 7, 8, 9, 10);
 
TaskHandle_t handlefan;
TaskHandle_t handlelr;
void TaskFan(void* parameters);
void TaskLR(void* parameters);
 
void setup() {
  stepper.setSpeed(5); // set the speed to 30 RPMs
  pinMode(fanPin, INPUT);
  pinMode(irPin, INPUT);
  pinMode(light, OUTPUT);
 
  xTaskCreate(TaskFan, "fan", 128, NULL, 1, &handlefan);
  xTaskCreate(TaskLR, "LR", 128, NULL, 1, &handlelr);
}
 
void loop() {
}
 
void TaskFan(void* parameters)
{
  (void) parameters;
  for (;;)
  {
    int fan = digitalRead(fanPin);
    if (fan == HIGH) stepper.step(360);
    vTaskDelay(1);
  }
  vTaskDelete(NULL);
}
 
void TaskLR(void* parameters)
{
  (void) parameters;
  for (;;)
  {
    int ir = analogRead(irPin);
    Serial.println(ir);
    if (ir < 750)
    {
      digitalWrite(light, HIGH);
    } else {
      digitalWrite(light, LOW);
    }
    vTaskDelay(100);
  }
 
  vTaskDelte(NULL);
}