/**
 * BleManager.cpp
 *
 * Implementarea clasei BleManager pentru Adaptive Traffic System
 *
 * Autor: Bulgariu (Mihăilă) Elena-Iuliana & Mihăilă Bogdan-Iulian
 */

#include "BleManager.h"
#include "DisplayManager.h"
#include "Config.h"
#include "TrafficAlertReceiver.h"

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
    BLEDevice::init(DEVICE_NAME);

    _pServer = BLEDevice::createServer();
    _pServer->setCallbacks(this);

    _pService = _pServer->createService(TRAFFIC_SIGN_SERVICE_UUID);

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

    _pService->start();

    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(TRAFFIC_SIGN_SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // Funcții pentru compatibilitate iOS
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    Serial.println("BLE server inițializat. Așteptăm conexiuni...");
    _pStatusCharacteristic->setValue("Ready");
}

void BleManager::sendStatusUpdate(const String& status) {
    if (_deviceConnected) {
        _pStatusCharacteristic->setValue(status.c_str());
        _pStatusCharacteristic->notify();
        Serial.print("Status notificat: ");
        Serial.println(status);
    }
}

void BleManager::onConnect(BLEServer* pServer) {
    _deviceConnected = true;
    Serial.println("Dispozitiv conectat!");
    sendStatusUpdate("Connected");
}

void BleManager::onDisconnect(BLEServer* pServer) {
    _deviceConnected = false;
    Serial.println("Dispozitiv deconectat!");

    // Repornește advertising când te deconectezi, pentru a permite noi conexiuni
    delay(500); // Delay mic pentru stabilizare
    pServer->startAdvertising();
    Serial.println("Restarting advertising");
}

void BleManager::onWrite(BLECharacteristic* characteristic) {
    if (characteristic->getUUID().toString() == SIGN_CHARACTERISTIC_UUID) {
        String command = characteristic->getValue().c_str(); // Conversia la const char* și apoi la String Arduino

        Serial.print("Comandă primită: ");
        Serial.println(command);

        // Verificare pentru comanda specială de resetare accident
        if (command == "RESET_ACCIDENT" || command == "RESET") {
            if (TrafficAlertReceiver::_instance) {
                TrafficAlertReceiver::_instance->resetAccidentState();

                // Trimitere confirmare că starea de accident a fost resetată
                sendStatusUpdate("Accident state reset");
            } else {
                sendStatusUpdate("Error: TrafficAlertReceiver not initialized");
            }
            return;
        }

        if (_displayManager) {
            _displayManager->showTrafficSign(command.c_str());

            sendStatusUpdate("Sign updated: " + command);
        } else {
            Serial.println("Error: Display Manager not initialized");
            sendStatusUpdate("Error: Display Manager not initialized");
        }
    }
}
