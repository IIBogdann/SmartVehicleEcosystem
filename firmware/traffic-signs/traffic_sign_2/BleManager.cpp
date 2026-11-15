/**
 * BleManager.cpp
 * 
 * Implementarea clasei BleManager pentru Adaptive Traffic System
 * 
 * Autor: Bulgariu (Mihăilă) Elena-Iulian
 */

#include "BleManager.h"
#include "DisplayManager.h"

BleManager::BleManager(DisplayManager* displayManager) : 
    _displayManager(displayManager),
    _deviceConnected(false),
    _oldDeviceConnected(false),
    _pServer(nullptr),
    _pService(nullptr),
    _pSignCharacteristic(nullptr),
    _pStatusCharacteristic(nullptr) {
}

void BleManager::init() {
    // Inițializare BLE
    BLEDevice::init("Traffic Sign");
    
    // Creare server BLE
    _pServer = BLEDevice::createServer();
    _pServer->setCallbacks(this);
    
    // Creare serviciu BLE
    _pService = _pServer->createService(TRAFFIC_SIGN_SERVICE_UUID);
    
    // Creare caracteristici BLE
    _pSignCharacteristic = _pService->createCharacteristic(
        SIGN_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    _pSignCharacteristic->setCallbacks(this);
    
    _pStatusCharacteristic = _pService->createCharacteristic(
        STATUS_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    _pStatusCharacteristic->addDescriptor(new BLE2902());
    
    // Start serviciu
    _pService->start();
    
    // Start advertising
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(TRAFFIC_SIGN_SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // Funcții pentru compatibilitate iOS
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    
    _pStatusCharacteristic->setValue("Ready");
}

void BleManager::sendStatusUpdate(const String& status) {
    if (_deviceConnected) {
        _pStatusCharacteristic->setValue(status.c_str());
        _pStatusCharacteristic->notify();
    }
}

void BleManager::onConnect(BLEServer* pServer) {
    _deviceConnected = true;
    sendStatusUpdate("Connected");
}

void BleManager::onDisconnect(BLEServer* pServer) {
    _deviceConnected = false;
    
    // Repornește advertising când te deconectezi, pentru a permite noi conexiuni
    delay(500); // Delay mic pentru stabilizare
    pServer->startAdvertising();
}

void BleManager::onWrite(BLECharacteristic* characteristic) {
    if (characteristic->getUUID().toString() == SIGN_CHARACTERISTIC_UUID) {
        // Obținem valoarea caracteristicii ca un șir de caractere
        String command = characteristic->getValue().c_str(); // Conversia la const char* și apoi la String Arduino
        
        // Procesează comanda primită
        if (_displayManager) {
            // Afișarea semnului corespunzător
            _displayManager->showTrafficSign(command.c_str());
            
            // Trimitere confirmare că semnul a fost afișat
            sendStatusUpdate("Sign updated: " + command);
        } else {
            sendStatusUpdate("Error: Display Manager not initialized");
        }
    }
}
