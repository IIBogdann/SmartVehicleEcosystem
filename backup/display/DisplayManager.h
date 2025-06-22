#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <GxEPD2_BW.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeMonoBold9pt7b.h>

class DisplayManager {
  public:
    // Constructor și inițializare
    DisplayManager();
    
    // Funcție principală de inițializare - include toate inițializările necesare
    void init();
    
    // Funcții principale
    void clear();
    void update();
    void hibernate();
    
    // Funcții de afișare complexe
    void welcomeMessage(); // Afișează mesajul de bun venit și inițiază secvența de tranziție
    void showTrafficSign(const char* sign); // Afișează un semn de trafic
    
    // Funcții de afișare detaliate
    void showWelcomeScreen(const char* title, const char* subtitle, const char* author, const char* date);
    void showStopSign();
    
    // Funcții utilitare
    void drawCenteredText(const char* text, int16_t y, uint8_t textSize);
    void clipire(int n); // Mutată din codul principal
    
  private:
    // Pinii pentru conexiunea cu e-paper
    static constexpr uint8_t PIN_BUSY = 5;    // Nu este folosit, dar îl declarăm
    static constexpr uint8_t PIN_RST  = 4;    // Reset
    static constexpr uint8_t PIN_DC   = 3;    // Data/Command
    static constexpr uint8_t PIN_CS   = 10;   // Chip Select
    static constexpr uint8_t PIN_SCK  = 6;    // Clock
    static constexpr uint8_t PIN_MOSI = 7;    // Master Out Slave In (MOSI/SDI)
    static constexpr int8_t  PIN_MISO = 2;    // Nu este necesar pentru scris
    
    // Referință la obiectul de display
    GxEPD2_BW<GxEPD2_213_flex, GxEPD2_213_flex::HEIGHT> display;
    
    // Metode de inițializare
    void initPins();
    void resetDisplay();
    void initSPI();
    void initDisplay();
};

extern DisplayManager epaperDisplay;

#endif // DISPLAY_MANAGER_H
