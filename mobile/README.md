# Aplicații Android

Cele două aplicații sunt proiecte Gradle independente și se deschid separat în Android Studio.

Din radacina repo-ului, `.\scripts\build.cmd android` compileaza ambele aplicatii.
Pentru una singura, foloseste `.\scripts\build.cmd vehicle-app` sau
`.\scripts\build.cmd signs-app`. Adauga `-Offline` daca ai deja dependintele in cache.

## `vehicle-app`

Aplicația mașinuței. Responsabilitate principală: Bogdan.

```powershell
cd mobile/vehicle-app
.\gradlew.bat test assembleDebug
```

## `traffic-signs-app`

Aplicația semnelor. Responsabilitate principală: Iuliana.

Pe lângă conexiunile BLE către semne, aplicația conține fluxul de scriere RFID. Deoarece writerul fizic este momentan montat pe mașinuță, aplicația deschide pentru această operație o conexiune SPP către Elysium.

```powershell
cd mobile/traffic-signs-app
.\gradlew.bat test assembleDebug
```

Rezultatele din `build/` și APK-urile nu se comit. Versiunile testate se publică prin GitHub Releases.
