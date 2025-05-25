#ifndef ULTRASONIC_SENSORS_H
#define ULTRASONIC_SENSORS_H

#include <Arduino.h>

// Definițiile pinilor
#define TRIG_FRONT 13
#define ECHO_FRONT 12
#define TRIG_BACK 14
#define ECHO_BACK 27
#define TRIG_LEFT 26
#define ECHO_LEFT 25
#define TRIG_RIGHT 33
#define ECHO_RIGHT 32
#define OBSTACLE_DISTANCE 30  // distanta de detectie

// Declară variabile externe
extern volatile long distanceFront;
extern volatile long distanceBack;
extern volatile long distanceLeft;
extern volatile long distanceRight;

// Funcții
void UltrasonicSensors_init();
long readDistanceCM(int trigPin, int echoPin);
void readSensorsSequentially();
String getSensorDataString();

#endif
