# Modulul Sensors

Acest director conține componentele responsabile pentru citirea senzorilor și colectarea datelor:

- **UltrasonicSensors.h/cpp**: Gestionează senzorii ultrasonici pentru măsurarea distanțelor
- **RFIDReader.h/cpp** (viitor): Va implementa citirea tag-urilor RFID pentru detectarea semnelor de circulație

Aceste componente sunt responsabile pentru:
- Inițializarea și configurarea senzorilor
- Citirea secvențială a senzorilor pentru evitarea interferențelor
- Procesarea datelor brute și convertirea în informații utile
- Expunerea valorilor măsurate către alte module
