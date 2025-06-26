/*
 * Acest fișier include toate implementările modulelor necesare pentru compilare
 * Este necesar deoarece Arduino IDE compilează automat doar fișierele din directorul sketch-ului
 */

// Core modules
// BluetoothManager este implementat direct în header, nu are fișier .cpp separat
#include "../core/TaskManager.cpp"

// Motion control
#include "../motion-control/DCMotor.cpp"
#include "../motion-control/ServoMotor.cpp"

// Sensors
#include "../sensors/UltrasonicSensors.cpp"
#include "../sensors/RFIDManager.cpp"

// Feedback
#include "../feedback/BuzzerManager.cpp"
#include "../alerts/AccidentDetector.cpp"
