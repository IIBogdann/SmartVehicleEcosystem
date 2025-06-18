#include "DisplayManager.h"
#include <SPI.h>

// LED pentru debugging
#define LED_BUILTIN 8

// Inițializare obiect global
DisplayManager epaperDisplay;

DisplayManager::DisplayManager() : 
  display(GxEPD2_213_flex(PIN_CS, PIN_DC, PIN_RST, /*busy*/ -1)) {
}

void DisplayManager::init() {
  Serial.begin(115200);
  while (!Serial) { delay(10); } // Așteaptă pentru serial pe unele plăci
  Serial.println("Inițializare E-Paper Display pentru Adaptive Traffic System");
  
  initPins();
  clipire(1);
  resetDisplay();
  initSPI();
  clipire(1);
  initDisplay();
  clipire(2);
  
  Serial.println(F("Display e-paper inițializat cu succes!"));
}

void DisplayManager::initPins() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_RST, OUTPUT);
  Serial.println(F("Pini configurați"));
}

void DisplayManager::resetDisplay() {
  Serial.println(F("Reset hardware display..."));
  digitalWrite(PIN_RST, LOW);
  delay(10);
  digitalWrite(PIN_RST, HIGH);
  delay(10);
}

void DisplayManager::initSPI() {
  Serial.println(F("Inițializare SPI..."));
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);
  // Optimizare viteză SPI
  SPI.setFrequency(10000000); // 10 MHz pentru viteză maximă
}

void DisplayManager::initDisplay() {
  Serial.println(F("Inițializare display e-paper..."));
  // Inițializare cu parametri optimizați pentru viteză
  display.init(0, false, 50); // Timeout mai mic de 50ms vs. default 10000ms
  display.setFullWindow();
  display.setPartialWindow(0, 0, display.width(), display.height());
  // Setare rotație corectă
  display.setRotation(3); // Landscape inversat (270°)
}

void DisplayManager::clear() {
  // Facem un clear mai agresiv 
  display.fillScreen(GxEPD_WHITE);
  display.clearScreen();
  display.fillScreen(GxEPD_WHITE); // A doua oară pentru a ne asigura
  delay(300); // Pauză mai lungă pentru stabilizare
}

void DisplayManager::update() {
  Serial.println(F("Actualizare display..."));
  display.display(true); // true = asincron, folosim timpul de updatare complet
  delay(100); // Așteptăm puțin pentru a porni actualizarea
}

void DisplayManager::hibernate() {
  Serial.println(F("Intrare în hibernare..."));
  display.hibernate();
}

// Metodă pentru afișarea completă a ecranului de bun venit
void DisplayManager::welcomeMessage() {
  // Datele pentru ecranul de bun venit
  showWelcomeScreen("Adaptive Traffic", "System", "Elena Mihaila", "09.06.2025");
  Serial.println(F("Inițializare completă! Afișare ecran de bun venit."));
  clipire(3);
}

// Metodă pentru afișarea unui semn de trafic specific
void DisplayManager::showTrafficSign(const char* sign) {
  Serial.print(F("Comandă primită pentru semn: "));
  Serial.println(sign);
  
  // Întâi facem un refresh complet pentru a șterge orice semn anterior
  clear();
  
  // Apoi procesăm comanda
  if (strcmp(sign, "STOP") == 0) {
    Serial.println(F("Procesare semn STOP"));
    showStopSign();
  } else if (strcmp(sign, "YIELD") == 0) {
    Serial.println(F("Procesare semn YIELD"));
    showYieldSign();
  } else if (strncmp(sign, "SPEED_LIMIT_", 12) == 0) {
    // Extrage limita de viteză din string (ex: "SPEED_LIMIT_30" -> 30)
    int limit = atoi(sign + 12);
    Serial.print(F("Procesare semn SPEED_LIMIT cu valoarea: "));
    Serial.println(limit);
    showSpeedLimitSign(limit);
  } else {
    Serial.println(F("Semn necunoscut!"));
    // Afișează un mesaj generic pentru semne necunoscute
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setTextSize(1);
    drawCenteredText("Semn necunoscut:", 20, 1);
    drawCenteredText(sign, 40, 2);
    update();
  }
  
  // Actualizăm display-ul cu noile date
  Serial.println(F("Semn afișat, display în hibernare."));
  clipire(5);
}

void DisplayManager::showWelcomeScreen(const char* title, const char* subtitle, 
                                     const char* author, const char* date) {
  Serial.println(F("Afișare ecran de bun venit..."));
  
  clear();
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  
  // Titlu principal
  display.setTextSize(2);
  drawCenteredText(title, 20, 2);
  
  // Subtitlu
  drawCenteredText(subtitle, 45, 2);
  
  // Separator
  int16_t lineY = 65;
  display.drawLine(10, lineY, display.width() - 10, lineY, GxEPD_BLACK);
  
  // Autor și dată
  display.setTextSize(1);
  display.setCursor(10, 80);
  display.println(author);
  
  display.setCursor(10, 95);
  display.println(date);
  
  update();
}

void DisplayManager::showStopSign() {
  Serial.println(F("Afișare semn STOP..."));
  
  clear();
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  
  // Desenăm textul STOP mare în centrul ecranului
  display.setTextSize(4);
  drawCenteredText("STOP", display.height() / 2, 4);
  
  update();
}

void DisplayManager::showYieldSign() {
  Serial.println(F("Afișare semn YIELD (CEDEAZĂ TRECEREA)..."));
  
  clear();
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  
  // Desenăm un triunghi simplu (reprezentând semnul de cedare a priorității)
  int16_t centerX = display.width() / 2;
  int16_t centerY = display.height() / 2;
  int16_t size = 40;
  
  // Desenează un triunghi
  display.drawTriangle(
    centerX, centerY - size,        // vârf
    centerX - size, centerY + size, // stânga jos
    centerX + size, centerY + size, // dreapta jos
    GxEPD_BLACK);
    
  // Desenează textul sub triunghi
  display.setTextSize(1);
  drawCenteredText("CEDA", centerY + size + 15, 2);
  drawCenteredText("TRECEREA", centerY + size + 30, 1);
  
  update();
}

void DisplayManager::showSpeedLimitSign(int limit) {
  Serial.print(F("Afișare semn LIMITĂ DE VITEZĂ: "));
  Serial.println(limit);
  
  clear();
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  
  // Desenăm un cerc pentru limita de viteză
  int16_t centerX = display.width() / 2;
  int16_t centerY = display.height() / 2;
  int16_t radius = 40;
  
  display.drawCircle(centerX, centerY, radius, GxEPD_BLACK);
  display.drawCircle(centerX, centerY, radius - 3, GxEPD_BLACK);
  
  // Afișăm limita de viteză
  display.setTextSize(3);
  
  // Convertim limita la un șir de caractere
  char limitText[10];
  sprintf(limitText, "%d", limit);
  
  drawCenteredText(limitText, centerY + 10, 3);
  
  // Afișăm "km/h" sub cerc
  display.setTextSize(1);
  drawCenteredText("km/h", centerY + radius + 15, 1);
  
  update();
}

void DisplayManager::drawCenteredText(const char* text, int16_t y, uint8_t textSize) {
  int16_t x1, y1;
  uint16_t w, h;
  
  display.setTextSize(textSize);
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  
  display.setCursor((display.width() - w) / 2, y);
  display.println(text);
}

// Funcție pentru clipirea LED-ului pentru debugging
void DisplayManager::clipire(int n) {
  for (int i = 0; i < n; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
  }
  delay(300); // Pauză între seturi de clipiri
}
