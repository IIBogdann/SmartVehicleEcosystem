// DisplayManager.cpp - implementarea clasei + obiectul global epaperDisplay
// Optimizat pentru dimensiune cod - 2025

#include "DisplayManager.h"
#include <SPI.h>
#include <Arduino.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 8
#endif

// ---------------------------- PARAMETRI CUSTOM -------------------------
constexpr int      CIRCLE_MARGIN        = 2;    // px
constexpr int      CIRCLE_BORDER_GAP    = 4;    // px
constexpr int      STOP_BORDER_GAP      = 5;
constexpr uint8_t  SPEED_TEXT_SIZE      = 5;    // 1‑8
constexpr int      SPEED_TEXT_V_ADJ     = 0;    // px
constexpr int      KMH_OFFSET           = 18;   // px
constexpr int16_t  YIELD_TRIANGLE_SIZE  = 45;   // px
constexpr int16_t  STOP_OCT_RADIUS      = 48;   // px (ajustează după ecran)
constexpr uint8_t  STOP_TEXT_SIZE       = 3;    // 1‑8
constexpr int      STOP_TEXT_V_ADJ      = 0;    // px (fine‑tune)
constexpr int     YIELD_BORDER_GAP    = 4;   // grosimea chenarului
constexpr uint8_t YIELD_TEXT_SIZE     = 1;   // mărime text sub semn

// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// DEFINIŢIA OBIECTULUI GLOBAL
// ----------------------------------------------------------------------
DisplayManager epaperDisplay;

// ----------------------------------------------------------------------
// CONSTRUCTOR
// ----------------------------------------------------------------------
DisplayManager::DisplayManager()
  : display(GxEPD2_213_flex(PIN_CS, PIN_DC, PIN_RST, /*busy*/ PIN_BUSY)) {}

// ----------------------------------------------------------------------
// INITIALIZARE
// ----------------------------------------------------------------------
void DisplayManager::init() {
  initPins();     clipire(1);
  resetDisplay();
  initSPI();      clipire(1);
  initDisplay();  clipire(2);
}

void DisplayManager::initPins() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_RST,     OUTPUT);
}

void DisplayManager::resetDisplay() {
  delay(500);
  pinMode(PIN_BUSY, INPUT_PULLUP);
  digitalWrite(PIN_RST, LOW);  delay(10);
  digitalWrite(PIN_RST, HIGH); delay(10);
}

void DisplayManager::initSPI() {
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);
  SPI.setFrequency(10000000);
}

void DisplayManager::initDisplay() {
  display.init(115200);
  display.setRotation(3);   // 3 = landscape flipped (180°)
}

// ----------------------------------------------------------------------
// UTILITARE
// ----------------------------------------------------------------------
void DisplayManager::clipire(int n) {
  for (int i = 0; i < n; ++i) {
    digitalWrite(LED_BUILTIN, HIGH); delay(200);
    digitalWrite(LED_BUILTIN, LOW);  delay(200);
  }
  delay(300);
}

void DisplayManager::drawCenteredText(const char* txt, int16_t baselineY, uint8_t sz) {
  int16_t x1, y1; uint16_t w, h;
  display.setTextSize(sz);
  display.getTextBounds(txt, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((display.width() - w) / 2, baselineY);
  display.println(txt);
}

// ----------------------------------------------------------------------
// REFRESH / CLEAR
// ----------------------------------------------------------------------
void DisplayManager::fullRefresh() {
  display.firstPage();
  do { display.fillScreen(GxEPD_WHITE); } while (display.nextPage());
  delay(50);
}

void DisplayManager::clear()       { fullRefresh(); }
void DisplayManager::clearScreen() { clear(); }
void DisplayManager::update() {
  // Implementare corectă a metodei update() pentru a actualiza display-ul
  // Folosește firstPage()/nextPage() pattern pentru refresh fără ghosting
  // display.display(true) ar fi o alternativă, dar cu firstPage/nextPage evităm ghostingul
  display.firstPage();
  do {
    // Conținutul este deja desenat prin alte metode
  } while (display.nextPage());
}
void DisplayManager::hibernate()   { display.hibernate(); }

// ----------------------------------------------------------------------
// ECRAN BUN VENIT
// ----------------------------------------------------------------------
void DisplayManager::welcomeMessage() {
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setTextSize(2);
    drawCenteredText("Indicator rutier adaptiv ", 20, 1);
    drawCenteredText("cu afisaj de tip E-Ink",           45, 1);
    int16_t lineY = 65;
    display.drawLine(10, lineY, display.width() - 10, lineY, GxEPD_BLACK);
    display.setTextSize(1);
    display.setCursor(10, 80);  display.println("Bulgariu Elena-Iuliana");
    display.setCursor(10, 95);  display.println("19.06.2025");
  } while (display.nextPage());
  clipire(3);
}

// ----------------------------------------------------------------------
// ROUTER SEMNE
// ----------------------------------------------------------------------
void DisplayManager::showTrafficSign(const char* sign) {
  if (strcmp(sign, "STOP") == 0)                     showStopSign();
  else if (strcmp(sign, "YIELD") == 0)               showYieldSign();
  else if (strncmp(sign, "SPEED_LIMIT_", 12) == 0)   showSpeedLimitSign(atoi(sign + 12));
  else {
    display.firstPage();
    do {
      display.fillScreen(GxEPD_WHITE);
      display.setTextColor(GxEPD_BLACK);
      display.setTextSize(1);
      drawCenteredText("Semn necunoscut:", 20, 1);
      drawCenteredText(sign,                 40, 2);
    } while (display.nextPage());
  }
}

// ----------------------------------------------------------------------
// SEMN STOP — OCTOGON CENTRAT
// ----------------------------------------------------------------------
void DisplayManager::showStopSign() {
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);

    // centru ecran
    const int16_t cx = display.width()  / 2;
    const int16_t cy = display.height() / 2;

    // raze exterior / interior
    const int16_t rOuter = STOP_OCT_RADIUS;
    const int16_t rInner = rOuter - STOP_BORDER_GAP;

    // ---------- OCTOGON EXTERIOR ----------
    int16_t k = (int16_t)round(rOuter / 1.41421356f);        // r/√2
    display.drawLine(cx - k, cy - rOuter, cx + k, cy - rOuter, GxEPD_BLACK);
    display.drawLine(cx + k, cy - rOuter, cx + rOuter, cy - k, GxEPD_BLACK);
    display.drawLine(cx + rOuter, cy - k, cx + rOuter, cy + k, GxEPD_BLACK);
    display.drawLine(cx + rOuter, cy + k, cx + k, cy + rOuter, GxEPD_BLACK);
    display.drawLine(cx + k, cy + rOuter, cx - k, cy + rOuter, GxEPD_BLACK);
    display.drawLine(cx - k, cy + rOuter, cx - rOuter, cy + k, GxEPD_BLACK);
    display.drawLine(cx - rOuter, cy + k, cx - rOuter, cy - k, GxEPD_BLACK);
    display.drawLine(cx - rOuter, cy - k, cx - k, cy - rOuter, GxEPD_BLACK);

    // ---------- OCTOGON INTERIOR (chenar dublu) ----------
    k = (int16_t)round(rInner / 1.41421356f);                // (r-Δ)/√2
    display.drawLine(cx - k, cy - rInner, cx + k, cy - rInner, GxEPD_BLACK);
    display.drawLine(cx + k, cy - rInner, cx + rInner, cy - k, GxEPD_BLACK);
    display.drawLine(cx + rInner, cy - k, cx + rInner, cy + k, GxEPD_BLACK);
    display.drawLine(cx + rInner, cy + k, cx + k, cy + rInner, GxEPD_BLACK);
    display.drawLine(cx + k, cy + rInner, cx - k, cy + rInner, GxEPD_BLACK);
    display.drawLine(cx - k, cy + rInner, cx - rInner, cy + k, GxEPD_BLACK);
    display.drawLine(cx - rInner, cy + k, cx - rInner, cy - k, GxEPD_BLACK);
    display.drawLine(cx - rInner, cy - k, cx - k, cy - rInner, GxEPD_BLACK);

    // ---------- TEXT „STOP” CENTRAT ----------
    const char* txt = "STOP";
    int16_t x1, y1; uint16_t w, h;
    display.setTextSize(STOP_TEXT_SIZE);
    display.getTextBounds(txt, 0, 0, &x1, &y1, &w, &h);

    int16_t baselineY = cy - (y1 + h / 2) + STOP_TEXT_V_ADJ; // perfect centrat
    display.setCursor(cx - w / 2, baselineY);
    display.print(txt);

  } while (display.nextPage());
}


// ----------------------------------------------------------------------
// SEMN YIELD (CEDAŢI TRECEREA)
// ----------------------------------------------------------------------
void DisplayManager::showYieldSign() {
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);

    const int16_t cx = display.width()  / 2;
    const int16_t cy = display.height() / 2;

    // --- triunghi exterior (vârf în JOS) ---
    int16_t s  = YIELD_TRIANGLE_SIZE;
    display.drawTriangle(
        cx - s, cy - s,   // colț stânga sus
        cx + s, cy - s,   // colț dreapta sus
        cx,     cy + s,   // vârf jos
        GxEPD_BLACK);

    // --- triunghi interior (chenar dublu) ---
    int16_t s2 = s - YIELD_BORDER_GAP;
    display.drawTriangle(
        cx - s2, cy - s2,
        cx + s2, cy - s2,
        cx,      cy + s2,
        GxEPD_BLACK);

    // --- text informativ sub semn (opțional) ---
    display.setTextSize(YIELD_TEXT_SIZE);
    drawCenteredText("CEDEAZĂ",  cy + s + 10, YIELD_TEXT_SIZE);
    drawCenteredText("TRECEREA", cy + s + 22, YIELD_TEXT_SIZE);

  } while (display.nextPage());
}

// ----------------------------------------------------------------------
// SEMN LIMITĂ VITEZĂ
// ----------------------------------------------------------------------
void DisplayManager::showSpeedLimitSign(int limit) {
  char buf[8]; sprintf(buf, "%d", limit);

  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);

    int16_t cx = display.width()  / 2;
    int16_t cy = display.height() / 2;
    int16_t r  = min(display.width(), display.height()) / 2 - CIRCLE_MARGIN;

    // Cercul dublu cu grosime ajustabilă
    display.drawCircle(cx, cy, r,                        GxEPD_BLACK);
    display.drawCircle(cx, cy, r - CIRCLE_BORDER_GAP,    GxEPD_BLACK);

    // Textul numărului
    int16_t x1, y1; uint16_t w, h;
    display.setTextSize(SPEED_TEXT_SIZE);
    display.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
    int16_t baselineY = cy - (y1 + h/2) + SPEED_TEXT_V_ADJ;
    drawCenteredText(buf, baselineY, SPEED_TEXT_SIZE);

    // "km/h" sub cerc
    display.setTextSize(1);
    drawCenteredText("km/h", cy + r + KMH_OFFSET, 1);

  } while (display.nextPage());
}
