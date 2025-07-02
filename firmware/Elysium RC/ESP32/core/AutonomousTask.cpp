#include "AutonomousTask.h"
#include "../motion-control/DCMotor.h"
#include "../motion-control/ServoMotor.h"
#include "TaskManager.h"
#include "../sensors/UltrasonicSensors.h"

// Flags and globals from main sketch
extern volatile bool stopMode;
extern volatile bool yieldMode;

// Flaguri pentru obstacole detectate
volatile bool rearObstacleDetected = false;
volatile bool frontObstacleDetected = false;
static unsigned long rearObstacleRemovalTime = 0;
static unsigned long frontObstacleRemovalTime = 0;
const int REAR_OBSTACLE_THRESHOLD = 30;  // cm
const int FRONT_OBSTACLE_THRESHOLD = 30; // cm
const int OBSTACLE_RESUME_DELAY = 3000;  // ms
extern int MOTOR_SPEED;        // defined in DCMotor.cpp

volatile OpMode currentMode = MODE_MANUAL;
volatile long arduinoCounter = 0;   // updated by Arduino over Serial1

// ---- Tuning constants ----
constexpr int AUTONOM_MOTOR_SPEED = 170;      // PWM duty for autonomous mode
constexpr long PULSES_PER_METER   = 1800;     // encoder pulses for 1 m
constexpr long PULSES_LONG  = 1200;           // 1 m forward
constexpr long PULSES_SHORT = 720;            // 0.4 m forward / backward
constexpr long TURN_LEFT_PULSES  = 1000;      // 90° left
constexpr long TURN_RIGHT_PULSES = 1200;      // 90° right
constexpr uint32_t PAUSE_MS = 1000;           // all pauses = 1 s

// ---- State machine ----
// Secvență exactă: 
// 1. inainte lung (LONG_FWD)
// 2. pauză (PAUSE1)
// 3. rotit roți stânga (STEER_LEFT)
// 4. pauză (PAUSE2)
// 5. viraj stânga (LEFT_TURN)
// 6. pauză (PAUSE3)
// 7. rotit roți dreapta (STEER_RIGHT)
// 8. pauză (PAUSE4)
// 9. viraj dreapta+marșarier (RIGHT_TURN) - cu encoder descrescător
// 10. pauză (PAUSE5)
// 11. centrare roți (STEER_CENTER)
// 12. pauză (PAUSE6)
// 13. înapoi la 1

enum AutoState {
  IDLE,
  LONG_FWD,  PAUSE1,
  STEER_LEFT, PAUSE2,
  LEFT_TURN, PAUSE3,
  STEER_RIGHT, PAUSE4,
  RIGHT_TURN, PAUSE5,
  STEER_CENTER, PAUSE6
};

static void autonomousTask(void *parameter){
  int prevMotorSpeed = MOTOR_SPEED;
  AutoState state   = IDLE;
  long startCnt     = 0;
  uint32_t phaseTs  = 0;

  for(;;){
    // manual override or yield/stop handling
    if(currentMode != MODE_AUTONOMOUS){
      // restore previous speed if we modified it
      if(MOTOR_SPEED != prevMotorSpeed)
        MOTOR_SPEED = prevMotorSpeed;
      // stop motors only once when leaving autonomous mode
      if(state != IDLE){
        DCMotor(false,false);
        state = IDLE;
      }
      vTaskDelay(pdMS_TO_TICKS(50));
      continue;
    }
    // Oprire la semn STOP - oprește imediat și așteaptă ieșirea din stopMode
    // Păstrează starea curentă pentru a relua exact de unde a rămas
    if(stopMode){
      // Oprire imediată, dar păstrează starea curentă și toate valorile
      DCMotor(false,false);
      
      // Nu centrăm roțile când e în stopMode pentru a păstra exact starea
      // și a putea continua mișcarea după expirarea stopMode
      
      // Doar așteptăm să expire stopMode
      vTaskDelay(pdMS_TO_TICKS(50));
      continue;
    }
    
    // Verificare obstacol în spate pentru mișcările în marșarier (RIGHT_TURN)
    if(state == RIGHT_TURN && distanceBack > 0 && distanceBack < REAR_OBSTACLE_THRESHOLD){
      // Obstacol în spate detectat
      if(!rearObstacleDetected){
        rearObstacleDetected = true;
        DCMotor(false,false);  // Oprește motoarele dar păstrează starea
        Serial.printf("[AUTO] Obstacol în spate detectat la %d cm - oprire\n", (int)distanceBack);
      }
      vTaskDelay(pdMS_TO_TICKS(100)); // Verificare mai rapidă pentru obstacole
      continue;
    } 
    
    // Verificare obstacol în față pentru mișcările înainte (LONG_FWD, LEFT_TURN)
    if((state == LONG_FWD || state == LEFT_TURN) && distanceFront > 0 && distanceFront < FRONT_OBSTACLE_THRESHOLD){
      // Obstacol în față detectat
      if(!frontObstacleDetected){
        frontObstacleDetected = true;
        DCMotor(false,false);  // Oprește motoarele dar păstrează starea
        Serial.printf("[AUTO] Obstacol în față detectat la %d cm - oprire\n", (int)distanceFront);
      }
      vTaskDelay(pdMS_TO_TICKS(100)); // Verificare mai rapidă pentru obstacole
      continue;
    }
    
    // Verifică dacă obstacolul din spate a fost îndepărtat
    if(rearObstacleDetected){
      if(distanceBack <= 0 || distanceBack >= REAR_OBSTACLE_THRESHOLD){
        // Obstacolul a fost îndepărtat, așteptăm 3 secunde înainte de a relua
        if(rearObstacleRemovalTime == 0){
          rearObstacleRemovalTime = millis();
          Serial.println("[AUTO] Obstacol spate îndepărtat - așteptăm 3 secunde...");
        }
        
        // Verifică dacă au trecut cele 3 secunde de la îndepărtarea obstacolului
        if(millis() - rearObstacleRemovalTime >= OBSTACLE_RESUME_DELAY){
          rearObstacleDetected = false;
          rearObstacleRemovalTime = 0;
          
          // Repornește motoarele în aceeași stare în care erau înainte
          if(state == RIGHT_TURN) {
            DCMotor(false, true); // Repornește marșarierul
            Serial.println("[AUTO] Reluare mișcare după obstacol spate - repornire marșarier");
          }
        } else {
          vTaskDelay(pdMS_TO_TICKS(50));
          continue;
        }
      } else {
        // Obstacolul încă este prezent
        vTaskDelay(pdMS_TO_TICKS(50)); 
        continue;
      }
    }
    
    // Verifică dacă obstacolul din față a fost îndepărtat
    if(frontObstacleDetected){
      if(distanceFront <= 0 || distanceFront >= FRONT_OBSTACLE_THRESHOLD){
        // Obstacolul a fost îndepărtat, așteptăm 3 secunde înainte de a relua
        if(frontObstacleRemovalTime == 0){
          frontObstacleRemovalTime = millis();
          Serial.println("[AUTO] Obstacol față îndepărtat - așteptăm 3 secunde...");
        }
        
        // Verifică dacă au trecut cele 3 secunde de la îndepărtarea obstacolului
        if(millis() - frontObstacleRemovalTime >= OBSTACLE_RESUME_DELAY){
          frontObstacleDetected = false;
          frontObstacleRemovalTime = 0;
          
          // Repornește motoarele în aceeași stare în care erau înainte
          if(state == LONG_FWD || state == LEFT_TURN) {
            DCMotor(true, false); // Repornește mișcarea înainte
            Serial.println("[AUTO] Reluare mișcare după obstacol față - repornire înainte");
          }
        } else {
          vTaskDelay(pdMS_TO_TICKS(50));
          continue;
        }
      } else {
        // Obstacolul încă este prezent
        vTaskDelay(pdMS_TO_TICKS(50)); 
        continue;
      }
    }
    if(!yieldMode && MOTOR_SPEED != AUTONOM_MOTOR_SPEED){
      prevMotorSpeed = MOTOR_SPEED;
      MOTOR_SPEED = AUTONOM_MOTOR_SPEED;
    }

    switch(state){
      case IDLE:
        ServoMotor(CENTER);
        startCnt = arduinoCounter;
        DCMotor(true,false);          // forward
        state = LONG_FWD;
        break;

      case LONG_FWD:
        if(arduinoCounter - startCnt >= PULSES_LONG){
          DCMotor(false,false);
          phaseTs = millis();
          state = PAUSE1;
        }
        break;

      case PAUSE1:
        if(millis() - phaseTs >= PAUSE_MS){
          ServoMotor(LEFT);
          phaseTs = millis();
          state = STEER_LEFT;         // wait 1s after steering
        }
        break;

      case STEER_LEFT:
        if(millis() - phaseTs >= PAUSE_MS){
          startCnt = arduinoCounter;
          DCMotor(true,false);        // pornește cu roți virate stânga
          state = LEFT_TURN;
        }
        break;

      case LEFT_TURN:
        if(arduinoCounter - startCnt >= TURN_LEFT_PULSES){
          DCMotor(false,false);
          phaseTs = millis();
          state = PAUSE3;             // direct la PAUSE3, fără SHORT_FWD
        }
        break;

      case PAUSE3:
        if(millis() - phaseTs >= PAUSE_MS){
          ServoMotor(RIGHT);
          phaseTs = millis();
          state = STEER_RIGHT;
        }
        break;

      case STEER_RIGHT:
        if(millis() - phaseTs >= PAUSE_MS){
          startCnt = arduinoCounter;
          DCMotor(false,true);        // deplasare în marșarier cu roți virate dreapta
          state = RIGHT_TURN;
        }
        break;

      case RIGHT_TURN:
        if(startCnt - arduinoCounter >= TURN_RIGHT_PULSES){  // encoderul descrește în marșarier
          DCMotor(false,false);
          phaseTs = millis();
          state = PAUSE5;
        }
        break;

      case PAUSE5:
        if(millis() - phaseTs >= PAUSE_MS){
          ServoMotor(CENTER);
          phaseTs = millis();
          state = STEER_CENTER;
        }
        break;

      case STEER_CENTER:
        if(millis() - phaseTs >= PAUSE_MS){
          phaseTs = millis();
          state = PAUSE6;
        }
        break;

      case PAUSE6:
        if(millis() - phaseTs >= PAUSE_MS){
          startCnt = arduinoCounter;
          DCMotor(true,false);        // begin next long forward
          state = LONG_FWD;
        }
        break;
    }

    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

void AutonomousTask_init(){
  xTaskCreatePinnedToCore(autonomousTask, "AutonomousTask", 4096, NULL, 1, NULL, 1);
}
