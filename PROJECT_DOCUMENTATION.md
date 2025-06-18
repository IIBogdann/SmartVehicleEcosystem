# Smart Vehicle Ecosystem - Proiect Documentație

## Descrierea Proiectului
Proiectul Smart Vehicle Ecosystem dezvoltă un sistem integrat de semne de circulație inteligente cu ecrane e-ink, controlate prin intermediul unei aplicații Android. Sistemul utilizează comunicarea BLE (Bluetooth Low Energy) între aplicația Android și modulele ESP32-C3, precum și comunicare ESP-NOW între diferitele module ESP32-C3 pentru schimbul autonom de informații.

## Componente Principale

### 1. Firmware ESP32-C3 (Semne Inteligente)
- **Locație:** `C:/Users/mbogd/Documents/SmartVehicleEcosystem/firmware/Adaptive Traffic System/EPaperDisplayTest/`
- **Hardware:** ESP32-C3 Super Mini cu display e-ink MH ET Live 2.13 inch (212×104 pixeli)

### 2. Aplicație Android (Controller)
- **Locație:** `C:/Users/mbogd/Documents/SmartVehicleEcosystem/mobile-app/smart_traffic_signs/`
- **Framework:** Kotlin, Jetpack Compose, BLE Android API

## Ce am Implementat

### Firmware ESP32-C3

#### 1. DisplayManager
- Interfața pentru controlul display-ului e-ink
- Funcții de desenare pentru diferite semne de circulație: STOP, YIELD (Cedează trecerea), SPEED_LIMIT (Limită de viteză)
- Optimizări pentru refresh-ul display-ului e-ink și evitarea ghosting-ului

#### 2. BleManager
- Server BLE pentru comunicarea cu aplicația Android
- Servicii și caracteristici definite pentru comenzi și notificări de status
- Procesare și executare comenzi primite de la aplicația Android

#### 3. Integrare în Fișierul Principal
- Inițializare și configurare DisplayManager și BleManager
- Rutină de afișare a ecranului de bun venit și semnului STOP inițial

### Aplicație Android

#### 1. Interfața Utilizator (Jetpack Compose)
- Ecran principal cu listarea dispozitivelor BLE disponibile
- Sistem de permisiuni pentru BLE (necesar pentru Android 12+)
- Comenzi de control pentru semnele de circulație

#### 2. Comunicare BLE
- Scanner BLE pentru descoperirea dispozitivelor
- Conectare și comunicare cu dispozitivele ESP32-C3
- Trimitere comenzi și primire notificări de status

## UUID-uri BLE (Sincronizate între ESP32 și Android)
- **Service UUID:** `"8f0e0d0c-0b0a-0908-0706-050403020100"`
- **Command Characteristic UUID:** `"8f0e0d0c-0b0a-0908-0706-050403020101"`
- **Status Characteristic UUID:** `"8f0e0d0c-0b0a-0908-0706-050403020102"`

## Fluxul de Comunicare
1. Aplicația Android scanează și descoperă dispozitivele BLE
2. Utilizatorul selectează dispozitivul "Traffic Sign"
3. Aplicația se conectează și trimite comenzi prin caracteristica Command
4. ESP32 primește comanda, actualizează display-ul și trimite confirmare prin caracteristica Status
5. Aplicația Android primește notificarea de status

## Comenzi Implementate
- `STOP` - Afișează semnul de oprire
- `YIELD` - Afișează semnul de cedare a trecerii
- `SPEED_LIMIT_XX` - Afișează o limită de viteză (ex: SPEED_LIMIT_30, SPEED_LIMIT_50)

## Ce Funcționează în Prezent
- ✅ Scanarea și descoperirea dispozitivelor BLE
- ✅ Conectarea la dispozitivul ESP32
- ✅ Trimiterea comenzilor pentru afișarea semnelor
- ✅ Afișarea semnelor STOP, YIELD și SPEED_LIMIT pe e-ink display
- ✅ Notificări de status de la ESP32 către aplicația Android

## Probleme Rezolvate
1. Erori de compilare legate de conversii între tipuri de string în BleManager
2. Ghosting (persistența imaginii) pe display-ul e-ink
3. Permisiuni BLE în Android 12+
4. Sincronizare UUID-uri între aplicație și ESP32

## Îmbunătățiri Viitoare

### Firmware ESP32
1. Implementarea mai multor tipuri de semne de circulație
2. Îmbunătățirea designului și layout-ului semnelor pe display
3. Adăugarea comunicării ESP-NOW între două plăci ESP32 pentru transferul autonom de informații
4. Optimizarea consumului de energie

### Aplicație Android
1. Îmbunătățirea interfeței utilizator
2. Adăugarea de funcții pentru configurație și personalizare
3. Implementarea unei logici de comandă mai complexe
4. Salvarea și restabilirea conexiunilor anterioare

### Integrare
1. Testare completă cu multiple dispozitive ESP32
2. Implementarea scenariilor de alertă automată între semne
3. Documentație tehnică detaliată pentru dezvoltare viitoare

## Instrucțiuni de Compilare și Instalare

### Firmware ESP32
1. Deschide proiectul în Arduino IDE
2. Instalează bibliotecile necesare: GxEPD2, BLE pentru ESP32, Adafruit GFX
3. Selectează placa ESP32-C3
4. Compilează și încarcă pe dispozitiv

### Aplicație Android
1. Deschide proiectul în Android Studio
2. Sincronizează Gradle
3. Compilează și instalează pe dispozitiv Android
4. Asigură-te că ai activat Bluetooth și Localizare

## Concluzii
Proiectul demonstrează posibilitatea controlului semnelor de circulație inteligente cu ecran e-ink prin intermediul unei aplicații Android, utilizând comunicare BLE. Sistemul actual reprezintă o dovadă de concept funcțională, cu potențial de dezvoltare pentru un ecosistem complet de semnalizare inteligentă.

---

*Documentație creată pentru Proiectul Smart Vehicle Ecosystem, Adaptive Traffic System*
*Data: 18 Iunie 2025*
