#ifndef BUZZER_MANAGER_H
#define BUZZER_MANAGER_H

#include <Arduino.h>

// Definițiile pinilor și constantelor
#define BUZZER_PIN 15
#define BUZZER_THRESHOLD 50

// Funcții
void Buzzer_init();
void tone(int pin, int frequency, int duration = 0);
void noTone(int pin);

#endif
