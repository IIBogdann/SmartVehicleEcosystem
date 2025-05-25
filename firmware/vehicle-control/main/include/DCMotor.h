#ifndef DC_MOTOR_H
#define DC_MOTOR_H

#include <Arduino.h>

// Definițiile pinilor și constantelor
extern const int PIN_MOTOR_IN1;
extern const int PIN_MOTOR_IN2;
extern const int PIN_MOTOR_ENA;
extern const int PWM_CHANNEL;
extern const int PWM_FREQ;
extern const int PWM_RESOLUTION;
extern const int MOTOR_SPEED;

// Variabile de stare
extern bool isMovingForward;
extern bool isMovingBackward;

// Funcții
void DCMotor_init();
void DCMotor(bool forward, bool backward);

#endif
