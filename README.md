# SmartVehicleEcosystem

## Descriere generală
SmartVehicleEcosystem este un sistem integrat pentru vehicule autonome și infrastructură inteligentă de semaforizare, dezvoltat ca proiect de licență. Sistemul combină un vehicul autonom controlat prin ESP32 și Arduino cu o infrastructură de semne de circulație bazate pe RFID și ecrane E-ink.

## Componente principale

### 1. Sistemul de control al vehiculului
- Platformă hardware bazată pe ESP32 și Arduino
- Control al mișcării prin motoare DC și servo
- Sistem de detecție a obstacolelor cu senzori ultrasonici
- Comunicare Bluetooth cu aplicația Android
- Capacitate de citire RFID pentru recunoașterea semnelor de circulație

### 2. Sistemul de semaforizare inteligentă
- Semne de circulație cu ecrane E-ink și ESP32 Mini C3
- Tag-uri RFID programabile cu informații de poziție și orientare
- Comunicare între indicatoare prin WiFi/Bluetooth
- Actualizare dinamică a conținutului în funcție de condițiile de trafic

### 3. Aplicația mobilă
- Interfață pentru controlul vehiculului
- Vizualizarea datelor senzoriale în timp real
- Monitorizarea stării sistemului

## Moduri de operare
- **MANUAL**: Control complet al utilizatorului prin aplicația Android
- **AUTO**: Control utilizator cu sisteme de siguranță active
- **AUTONOMOUS**: Funcționare autonomă bazată pe trasee înregistrate

## Structura proiectului

```
SmartVehicleEcosystem/
├── firmware/                      # Codul pentru microcontrolere
│   ├── vehicle-control/           # Controlul vehiculului
│   └── traffic-signs/             # Sistemul de semne de circulație
├── mobile-app/                    # Aplicația Android
├── hardware/                      # Documentație hardware
├── docs/                          # Documentație detaliată
└── tools/                         # Unelte și utilitare
```

## Instalare și configurare

### Cerințe
- Arduino IDE sau PlatformIO
- Android Studio pentru aplicația mobilă
- ESP32 și componente hardware conform listei din documentație

### Compilare și încărcare
1. Deschide proiectul vehiculului în Arduino IDE/PlatformIO
2. Selectează placa ESP32
3. Compilează și încarcă codul pe dispozitiv

## Licență
Proiect dezvoltat în scop educațional de IIBogdann și Mihaila.

## Contribuții
Proiect realizat de:
- IIBogdann: Sistemul de control al vehiculului
- Mihaila: Sistemul de semaforizare inteligentă