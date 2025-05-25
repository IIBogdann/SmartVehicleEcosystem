#ifndef SERVO_MOTOR_H
#define SERVO_MOTOR_H

#include <Arduino.h>
#include <ESP32Servo.h>

// În ServoControl.h
#define LEFT 0
#define CENTER 38
#define RIGHT 80
#define SERVO_PIN 22


extern Servo servo;
extern int currentServoAngle;


// Funcții
void ServoMotor_init();
void ServoMotor(int position);


#endif
