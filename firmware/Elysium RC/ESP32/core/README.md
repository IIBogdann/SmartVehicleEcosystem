# Modulul Core

Acest director conține componentele de bază pentru funcționarea sistemului Elysium RC:

- **BluetoothManager.h/cpp**: Gestionează comunicarea Bluetooth cu aplicația Android
- **TaskManager.h/cpp**: Implementează sistemul de taskuri FreeRTOS și coordonează comunicarea între componente

Aceste componente sunt responsabile pentru:
- Inițializarea taskurilor FreeRTOS
- Gestionarea cozii de mesaje
- Comunicarea cu dispozitivele externe
- Coordonarea între celelalte module
