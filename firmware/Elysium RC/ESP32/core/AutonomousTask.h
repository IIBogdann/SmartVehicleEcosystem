#pragma once

#include <Arduino.h>

// Inițializează task-ul autonom; trebuie apelat din setup() după inițierea perifericelor
void AutonomousTask_init();

// Enumerarea modurilor de operare (expusă global pentru comenzi BLE)
enum OpMode { MODE_MANUAL, MODE_AUTONOMOUS };
extern volatile OpMode currentMode;

// Contorul de pulsuri primit de la Arduino UNO (encoder)
extern volatile long arduinoCounter;
