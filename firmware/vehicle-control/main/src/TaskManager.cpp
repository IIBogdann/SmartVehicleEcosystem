#include "TaskManager.h"
#include "UltrasonicSensors.h"
#include "BuzzerManager.h"

// Declarăm coada pentru mesaje
QueueHandle_t queue;

void Tasks_init() {
  Serial.println("\nCreare coadă mesaje...");
  queue = xQueueCreate(5, sizeof(char[50]));
  
  Serial.println("\nCreare taskuri...");
  xTaskCreate(
    obstacleDetectionTask, 
    "ObstacleTask", 
    2048,           // Stack size
    NULL,           // Task input parameter
    1,              // Priority
    NULL            // Task handle
  );
  
  xTaskCreate(
    buzzerTask, 
    "BuzzerTask", 
    2048,           // Stack size
    NULL,           // Task input parameter
    1,              // Priority
    NULL            // Task handle
  );
}

void obstacleDetectionTask(void *parameter) {
  while (true) {
    // Folosim variabilele globale care sunt actualizate ciclic în loop()
    long front = distanceFront;
    long back  = distanceBack;
    long left  = distanceLeft;
    long right = distanceRight;

    // Formăm un mesaj de diagnosticare
    String mesajComplet = String("Obstacle Task: ") +
                        "Front: " + String(front) + " " +
                        "Back: " + String(back) + " " +
                        "Left: " + String(left) + " " +
                        "Right: " + String(right);
                        
    Serial.println(mesajComplet);
    
    // Activăm buzzerul dacă obstacolul din față este prea aproape
    if (front > 0 && front < OBSTACLE_DISTANCE) {
      digitalWrite(BUZZER_PIN, HIGH);
    } else {
      digitalWrite(BUZZER_PIN, LOW);
    }
    
    // IMPORTANT: taskul nu trebuie să se termine niciodată!
    vTaskDelay(pdMS_TO_TICKS(500));  // Pauză de 500 ms între verificări
  }
  
  // Nu se ajunge niciodată aici
  vTaskDelete(NULL);
}

void buzzerTask(void *parameter) {
  // Parametrii pentru temporizare:
  const int minDelay = 50;    // întârziere minimă (ms) când obstacolul este foarte aproape
  const int maxDelay = 500;   // întârziere maximă (ms) când obstacolul este la limita pragului
  const int beepOnTime = 50;  // durata efectivă a bipului (ms)
  
  // Pentru calculul ratei de apropiere (viteză aparentă)
  long previousMinDistance = BUZZER_THRESHOLD;
  
  while (true) {
    // Citim valorile locale ale senzorilor
    long front = distanceFront;
    long left  = distanceLeft;
    long right = distanceRight;
    long back  = distanceBack;
    
    // Determinăm senzorul cu obstacolul cel mai apropiat (sub BUZZER_THRESHOLD)
    int sensorID = -1;   // 0: front, 1: stânga, 2: dreapta, 3: spate
    long minDistance = BUZZER_THRESHOLD + 1; // valoare inițială peste prag
    
    if (front > 0 && front < BUZZER_THRESHOLD && front < minDistance) {
      minDistance = front;
      sensorID = 0;
    }
    if (left > 0 && left < BUZZER_THRESHOLD && left < minDistance) {
      minDistance = left;
      sensorID = 1;
    }
    if (right > 0 && right < BUZZER_THRESHOLD && right < minDistance) {
      minDistance = right;
      sensorID = 2;
    }
    if (back > 0 && back < BUZZER_THRESHOLD && back < minDistance) {
      minDistance = back;
      sensorID = 3;
    }
    
    if (sensorID != -1) {
      int toneFreq = 0;
      // Asociem fiecărui senzor o frecvență unică:
      switch(sensorID) {
        case 0: toneFreq = 1000; break; // față: 1000 Hz
        case 1: toneFreq = 800;  break; // stânga: 800 Hz
        case 2: toneFreq = 1200; break; // dreapta: 1200 Hz
        case 3: toneFreq = 600;  break; // spate: 600 Hz
      }

      if (minDistance < 4) {
        tone(BUZZER_PIN, toneFreq);  // pornește tonul continuu
        // Așteptăm puțin înainte de re-verificare
        vTaskDelay(pdMS_TO_TICKS(100));
      } else {
        // Calculăm timpul de pauză între bipuri folosind o mapare cvadratică:
        int offTime = minDelay + ((minDistance * minDistance * (maxDelay - minDelay)) / (BUZZER_THRESHOLD * BUZZER_THRESHOLD));
        
        // Calculăm „viteza" de apropiere (diferența între citirile succesive)
        long speed = previousMinDistance - minDistance; // pozitiv dacă obiectul se apropie
        previousMinDistance = minDistance;
        if (speed > 5) {  
           // Dacă obiectul se apropie rapid (mai mult de 5 cm între citiri), reducem și mai mult offTime
           offTime = offTime / 2;
        }
        
        // Emit bipul:
        tone(BUZZER_PIN, toneFreq, beepOnTime);
        vTaskDelay(pdMS_TO_TICKS(beepOnTime));
        noTone(BUZZER_PIN);
        vTaskDelay(pdMS_TO_TICKS(offTime));
      }
    } else {
      // Niciun obstacol detectat: ne asigurăm că buzzer-ul este oprit
      noTone(BUZZER_PIN);
      previousMinDistance = BUZZER_THRESHOLD; // resetăm pentru următoarea comparație
      vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    // IMPORTANT: adăugăm un mic delay între verificări
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  
  // Nu se ajunge niciodată aici
  vTaskDelete(NULL);
}
