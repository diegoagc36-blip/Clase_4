#include <Arduino.h>

const int BUTTON_PIN = 4;

volatile bool emergencyTriggered = false;
volatile uint32_t totalPressCount = 0;
int systemState = 0;

void IRAM_ATTR goodButtonISR() {
emergencyTriggered = true;
totalPressCount++;
}

void setup() {
Serial.begin(115200);
pinMode(BUTTON_PIN, INPUT_PULLUP);
attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), goodButtonISR, FALLING);
}

void loop() {
if (totalPressCount > 100) {
Serial.println("Maintenance required");
}

if (systemState == 0) {
if (emergencyTriggered) {
Serial.println("Interrupt handled safely in loop!");
systemState = 1;
emergencyTriggered = false;
}
} else if (systemState == 1) {
systemState = 0;
}
}