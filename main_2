#include <Arduino.h>
#include "xtensa/core-macros.h"


constexpr uint8_t BUTTON_PIN = 4;
constexpr uint32_t DEBOUNCE_CYCLES = 4000000UL;

enum class State : uint8_t { IDLE = 0, ACTIVE, FAULT_LOCKOUT, COUNT };
enum class Event : uint8_t { NONE = 0, BUTTON_PRESSED, RESET_TIMEOUT, COUNT };


volatile uint32_t isrLastCycle = 0;
volatile uint32_t isrPressCount = 0;
volatile bool isrEventFlag = false;
portMUX_TYPE isrMux = portMUX_INITIALIZER_UNLOCKED;


void IRAM_ATTR buttonISR() {
uint32_t currentCycles = XTHAL_GET_CCOUNT();
if ((currentCycles - isrLastCycle) >= DEBOUNCE_CYCLES) {
isrLastCycle = currentCycles;

portENTER_CRITICAL_ISR(&isrMux);
isrPressCount++;
isrEventFlag = true;
portEXIT_CRITICAL_ISR(&isrMux);
}
}


void onEnterActive();
void onEnterFault();
void onEnterIdle();
void onNoOp();


struct Transition {
State nextState;
void (*action)();
};

constexpr Transition FSM_TABLE[static_cast<size_t>(State::COUNT)]
[static_cast<size_t>(Event::COUNT)] = {

{ {State::IDLE, onNoOp}, {State::ACTIVE, onEnterActive}, {State::IDLE, onNoOp} },

{ {State::ACTIVE, onNoOp}, {State::FAULT_LOCKOUT, onEnterFault}, {State::IDLE, onEnterIdle} },

{ {State::FAULT_LOCKOUT, onNoOp}, {State::FAULT_LOCKOUT, onNoOp}, {State::IDLE, onEnterIdle} }
};


State currentState = State::IDLE;
uint32_t stateTimerMs = 0;

void onNoOp() {}
void onEnterActive() {
stateTimerMs = millis();
Serial.println("[FSM] -> ACTIVE");
}
void onEnterFault() {
stateTimerMs = millis();
Serial.println("[FSM] -> FAULT_LOCKOUT");
}
void onEnterIdle() {
stateTimerMs = millis();
Serial.println("[FSM] -> IDLE");
}

void setup() {
Serial.begin(115200);
pinMode(BUTTON_PIN, INPUT_PULLUP);
attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);
}

void loop() {

Event currentEvent = Event::NONE;
uint32_t currentCount = 0;


portENTER_CRITICAL(&isrMux);
if (isrEventFlag) {
isrEventFlag = false;
currentEvent = Event::BUTTON_PRESSED;
}
currentCount = isrPressCount;
portEXIT_CRITICAL(&isrMux);


if (millis() - stateTimerMs >= 5000UL) {
currentEvent = Event::RESET_TIMEOUT;
}



if (currentEvent != Event::NONE) {
const Transition& t = FSM_TABLE[static_cast<size_t>(currentState)]
[static_cast<size_t>(currentEvent)];
currentState = t.nextState;
t.action();

Serial.printf("[Metrics] Microswitch actuations: %lu\n",
static_cast<unsigned long>(currentCount));
}
}
