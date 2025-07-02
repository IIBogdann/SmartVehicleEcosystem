#include "TaskManager.h"
#include "../sensors/UltrasonicSensors.h"
#include "../feedback/BuzzerManager.h"

// Accesăm flag-ul definit în sketch-ul principal pentru modul de scriere tag
extern volatile bool rfidWriteMode;
extern volatile unsigned long muteConsoleUntil;

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
  

}

void obstacleDetectionTask(void *parameter) {
  while (true) {
    // Folosim variabilele globale care sunt actualizate ciclic în loop()
    long front = distanceFront;
    long back  = distanceBack;
    long left  = distanceLeft;
    long right = distanceRight;

    // Dacă suntem în modul de scriere tag, nu aglomerăm consola
    if(millis() >= muteConsoleUntil){
      String mesajComplet = String("Obstacle Task: ") +
                          "Front: " + String(front) + " " +
                          "Back: " + String(back) + " " +
                          "Left: " + String(left) + " " +
                          "Right: " + String(right);
      ///Serial.println(mesajComplet);
    }
    
    // Buzzer gestionat de BuzzerManager → nu mai controlăm direct pinul aici
    
    // IMPORTANT: taskul nu trebuie să se termine niciodată!
    vTaskDelay(pdMS_TO_TICKS(500));  // Pauză de 500 ms între verificări
  }
  
  // Nu se ajunge niciodată aici
  vTaskDelete(NULL);
}


