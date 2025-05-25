#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// Declarări externe
extern QueueHandle_t queue;

// Funcții de task
void buzzerTask(void *parameter);
void obstacleDetectionTask(void *parameter);

// Funcții de management
void Tasks_init();

#endif
