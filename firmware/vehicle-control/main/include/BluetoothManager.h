#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include <BluetoothSerial.h>

class BluetoothManager {
private:
    BluetoothSerial SerialBT;
    const char* deviceName = "ElysiumRC";
    
public:
    void begin() {
        SerialBT.begin(deviceName);
        Serial.println("Bluetooth device started, you can pair it with your Android device!");
    }
    
    bool isConnected() {
        return SerialBT.connected();
    }
    
    void sendData(const String& data) {
        if (isConnected()) {
            SerialBT.println(data);
        }
    }
    
    String receiveData() {
        String data = "";
        if (SerialBT.available()) {
            data = SerialBT.readStringUntil('\n');
            data.trim();
        }
        return data;
    }
};

#endif
