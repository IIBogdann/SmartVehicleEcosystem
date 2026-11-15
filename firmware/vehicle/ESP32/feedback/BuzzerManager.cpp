#include "BuzzerManager.h"
#include "../sensors/UltrasonicSensors.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ---- Config constant & forward declarations ----
static const int BUZZER_CHANNEL = 3;           // PWM channel for buzzer
static TaskHandle_t buzzerTaskHandle = nullptr;
static void buzzerTaskRunner(void *parameter);


void Buzzer_init() {
  Serial.println("\nConfigurare buzzer...");
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);


  // porneşte taskul de gestiune buzzer dacă nu există
  if (buzzerTaskHandle == nullptr) {
    xTaskCreate(buzzerTaskRunner, "BuzzerTask", 2048, NULL, 1, &buzzerTaskHandle);
  }
}


void tone(int /*pin*/, int frequency, int duration) {
  // Atașăm sau actualizăm canalul PWM cu frecvența dorită (rezoluție 8-bit)
  ledcAttachChannel(BUZZER_PIN, frequency, 8, BUZZER_CHANNEL);
  // Duty 50 % – valoare 128 la rezoluție 8-bit
  ledcWrite(BUZZER_PIN, 128);

  if (duration > 0) {
    delay(duration);
    noTone(BUZZER_PIN);
  }
}

void noTone(int /*pin*/) {
  // Oprește semnalul PWM
  ledcWrite(BUZZER_PIN, 0);
}

/* ---------- logica de parking-assist MUTATĂ într-un task FreeRTOS ---------- */

static void buzzerTaskRunner(void *parameter) {
	
	 Serial.println("\BUZZER TASK WORNING  buzzer...");
	
  const int minDelay = 50;   // ms
  const int maxDelay = 500;  // ms
  const int beepOnTime = 50; // ms durată puls

  long previousMinDistance = BUZZER_THRESHOLD;

  for (;;) {
    long front  = distanceFront;
    long left   = distanceLeft;
    long right  = distanceRight;
    long back   = distanceBack;

    int sensorID = -1; // 0 F,1 L,2 R,3 B
    long minDistance = BUZZER_THRESHOLD + 1;
    if (front > 0 && front < BUZZER_THRESHOLD && front < minDistance) { minDistance = front; sensorID = 0; }
    if (left  > 0 && left  < BUZZER_THRESHOLD && left  < minDistance) { minDistance = left;  sensorID = 1; }
    if (right > 0 && right < BUZZER_THRESHOLD && right < minDistance) { minDistance = right; sensorID = 2; }
    if (back  > 0 && back  < BUZZER_THRESHOLD && back  < minDistance) { minDistance = back;  sensorID = 3; }

    if (sensorID != -1) {
      const int SENSOR_TONES[4] = {1000, 800, 1200, 600};
      int toneFreq = SENSOR_TONES[sensorID];

      if (minDistance < 4) {
        tone(BUZZER_PIN, toneFreq); // continuu
        vTaskDelay(pdMS_TO_TICKS(100));
      } else {
        int offTime = minDelay + ((minDistance * minDistance * (maxDelay - minDelay)) / (BUZZER_THRESHOLD * BUZZER_THRESHOLD));
        long speed = previousMinDistance - minDistance;
        previousMinDistance = minDistance;
        if (speed > 5) offTime /= 2;

        tone(BUZZER_PIN, toneFreq, beepOnTime);
        vTaskDelay(pdMS_TO_TICKS(beepOnTime + offTime));
      }
    } else {
      noTone(BUZZER_PIN);
      previousMinDistance = BUZZER_THRESHOLD;
      vTaskDelay(pdMS_TO_TICKS(100));
    }
  }
}

// vechiul Buzzer_update rămâne gol (nu mai este folosit)
void Buzzer_update() {}


