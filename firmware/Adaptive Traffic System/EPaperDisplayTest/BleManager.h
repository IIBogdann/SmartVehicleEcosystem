/**
 * BleManager.h
 * 
 * Gestionează comunicarea BLE pentru Adaptive Traffic System
 * Permite controlul display-ului e-paper prin BLE de pe aplicația Android
 * 
 * Autor: Bulgariu (Mihăilă) Elena-Iuliana & Mihăilă Bogdan-Iulian
 */

#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Clasa pentru gestionarea display-ului (forward declaration)
class DisplayManager;

// UUID-uri pentru servicii și caracteristici BLE
// IMPORTANT: Aceste UUID-uri trebuie să fie identice cu cele din aplicația Android
#define TRAFFIC_SIGN_SERVICE_UUID        "8f0e0d0c-0b0a-0908-0706-050403020100"
#define SIGN_CHARACTERISTIC_UUID         "8f0e0d0c-0b0a-0908-0706-050403020101"
#define STATUS_CHARACTERISTIC_UUID       "8f0e0d0c-0b0a-0908-0706-050403020102"

class BleManager : public BLEServerCallbacks, public BLECharacteristicCallbacks {
public:
    BleManager(DisplayManager* displayManager);
    void init();
    void sendStatusUpdate(const String& status);
    
    // Metode de callback pentru BLEServerCallbacks
    void onConnect(BLEServer* pServer) override;
    void onDisconnect(BLEServer* pServer) override;
    
    // Metode de callback pentru BLECharacteristicCallbacks
    void onWrite(BLECharacteristic *characteristic) override;
    
private:
    DisplayManager* _displayManager;
    BLEServer* _pServer;
    BLEService* _pService;
    BLECharacteristic* _pSignCharacteristic;
    BLECharacteristic* _pStatusCharacteristic;
    bool _deviceConnected;
    bool _oldDeviceConnected;
};

#endif // BLE_MANAGER_H
