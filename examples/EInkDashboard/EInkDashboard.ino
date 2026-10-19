/*
  EInkDashboard.ino - Electronic Paper (E-Ink) System Display Emulator
  Part of the ArduinoUnoQ_AdafruitTouch library.
  
  Features:
  - Realistic monochrome & grayscale E-Paper aesthetic with paper-white background
  - Simulated E-Ink flash refresh on page transitions
  - Complete technical specifications of the Arduino UNO Q (STM32U585 + Qualcomm QRB2210)
  - Wi-Fi (802.11ac dual-band) & BLE 5.0 wireless hardware specs and bridge details
  - 3 Interactive E-Paper pages:
      Page 1: System & Dual-Core Architecture Overview
      Page 2: Wireless & Network (Wi-Fi, Bluetooth BLE 5.0, Bridge)
      Page 3: Pinout, Hardware Interfaces & Adafruit Shield Status
  - Touch-driven page navigation and manual E-Paper refresh button
  
  Board: Arduino UNO Q (STM32U585)
  Shield: Adafruit 2.8" Capacitive TFT (PID 1947)
  Author: ankur608
*/

#include <ArduinoUnoQ_AdafruitTouch.h>

UnoQ_AdafruitTouch screen;

#define SCREEN_W 240
#define SCREEN_H 320

// Authentic E-Ink Color Palette
#define EINK_WHITE       0xFFFF  // Pure paper background
#define EINK_BLACK       0x0000  // High-contrast ink
#define EINK_LIGHT_GRAY  0xD69A  // Stippled secondary tint
#define EINK_DARK_GRAY   0x6B4D  // Charcoal rule lines
#define EINK_BORDER      0x2965  // Deep slate outline

int currentPage = 0;
const int TOTAL_PAGES = 3;

// Bottom Navigation Bar Coordinates
#define NAV_Y       282
#define NAV_H        34
#define BTN_PREV_X    6
#define BTN_PREV_W   68
#define BTN_REF_X    78
#define BTN_REF_W    84
#define BTN_NEXT_X  166
#define BTN_NEXT_W   68

// ==============================================================================
// SIMULATED E-INK REFRESH EFFECT (Black/White flash like an actual e-paper panel)
// ==============================================================================
void triggerEInkRefresh() {
  // 1. Invert to dark (clears ghosting)
  screen.tft.fillScreen(EINK_BLACK);
  delay(35);
  // 2. White flash
  screen.tft.fillScreen(EINK_WHITE);
  delay(45);
}

// ==============================================================================
// E-PAPER HEADER & STATUS ICONS (Wi-Fi, BLE, Battery, Clock)
// ==============================================================================
void drawEInkHeader() {
  screen.tft.fillRect(0, 0, SCREEN_W, 28, EINK_WHITE);
  
  // Title / Brand
  screen.tft.setTextColor(EINK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(8, 9);
  screen.tft.print("ARDUINO UNO Q");

  // Version badge
  screen.tft.fillRect(92, 7, 28, 12, EINK_BLACK);
  screen.tft.setTextColor(EINK_WHITE);
  screen.tft.setCursor(95, 9);
  screen.tft.print("V1.0");

  // Status Icons (Right side)
  // 1. Wi-Fi Icon (Arced signal bars)
  int wx = 142, wy = 15;
  screen.tft.drawCircle(wx, wy, 2, EINK_BLACK);
  screen.tft.drawCircle(wx, wy, 5, EINK_BLACK);
  screen.tft.drawCircle(wx, wy, 8, EINK_BLACK);
  screen.tft.fillRect(wx - 9, wy + 1, 18, 10, EINK_WHITE); // Clip lower half

  // 2. BLE Icon (Runic B)
  int bx = 168, by = 6;
  screen.tft.drawFastVLine(bx + 4, by, 14, EINK_BLACK);
  screen.tft.drawLine(bx + 4, by + 7, bx + 9, by + 3, EINK_BLACK);
  screen.tft.drawLine(bx + 9, by + 3, bx + 1, by + 11, EINK_BLACK);
  screen.tft.drawLine(bx + 4, by + 7, bx + 9, by + 11, EINK_BLACK);
  screen.tft.drawLine(bx + 9, by + 11, bx + 1, by + 3, EINK_BLACK);

  // 3. Battery Icon
  int rx = 196, ry = 7;
  screen.tft.drawRect(rx, ry, 26, 12, EINK_BLACK);
  screen.tft.fillRect(rx + 26, ry + 3, 2, 6, EINK_BLACK);
  screen.tft.fillRect(rx + 2, ry + 2, 18, 8, EINK_BLACK); // 85% charge

  // Divider Line
  screen.tft.drawFastHLine(6, 26, SCREEN_W - 12, EINK_BLACK);
  screen.tft.drawFastHLine(6, 27, SCREEN_W - 12, EINK_DARK_GRAY);
}

// ==============================================================================
// NAVIGATION DOCK
// ==============================================================================
void drawEInkNav() {
  // Divider above navigation
  screen.tft.drawFastHLine(6, NAV_Y - 5, SCREEN_W - 12, EINK_DARK_GRAY);

  // PREV Button
  screen.tft.drawRoundRect(BTN_PREV_X, NAV_Y, BTN_PREV_W, NAV_H, 4, EINK_BLACK);
  screen.tft.setTextColor(EINK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(BTN_PREV_X + 14, NAV_Y + 12);
  screen.tft.print("< PREV");

  // REFRESH Button
  screen.tft.fillRoundRect(BTN_REF_X, NAV_Y, BTN_REF_W, NAV_H, 4, EINK_BLACK);
  screen.tft.setTextColor(EINK_WHITE);
  screen.tft.setCursor(BTN_REF_X + 16, NAV_Y + 12);
  screen.tft.print("REFRESH");

  // NEXT Button
  screen.tft.drawRoundRect(BTN_NEXT_X, NAV_Y, BTN_NEXT_W, NAV_H, 4, EINK_BLACK);
  screen.tft.setTextColor(EINK_BLACK);
  screen.tft.setCursor(BTN_NEXT_X + 14, NAV_Y + 12);
  screen.tft.print("NEXT >");
}

// ==============================================================================
// CARD / PANEL BUILDER
// ==============================================================================
void drawEInkCard(int x, int y, int w, int h, const char* title) {
  // Card outer frame
  screen.tft.drawRoundRect(x, y, w, h, 4, EINK_BLACK);
  
  // Card title tab
  screen.tft.fillRect(x + 6, y - 5, strlen(title) * 6 + 10, 11, EINK_WHITE);
  screen.tft.setTextColor(EINK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(x + 10, y - 4);
  screen.tft.print(title);
}

// ==============================================================================
// PAGE 1: DUAL-CORE HARDWARE ARCHITECTURE
// ==============================================================================
void drawPage1_Architecture() {
  // Page Title Bar
  screen.tft.fillRect(8, 32, SCREEN_W - 16, 16, EINK_LIGHT_GRAY);
  screen.tft.drawRect(8, 32, SCREEN_W - 16, 16, EINK_BLACK);
  screen.tft.setTextColor(EINK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(14, 36);
  screen.tft.print("PAGE 1/3: DUAL-CORE HARDWARE SPECS");

  // Card 1: Microcontroller (MCU Core)
  drawEInkCard(8, 60, SCREEN_W - 16, 95, "REAL-TIME MCU (STM32U585)");
  screen.tft.setCursor(16, 70);
  screen.tft.print("* Core: Arm Cortex-M33 @ 160MHz");
  screen.tft.setCursor(16, 82);
  screen.tft.print("* Security: Arm TrustZone Support");
  screen.tft.setCursor(16, 94);
  screen.tft.print("* Non-Volatile Flash: 2048 KB (2MB)");
  screen.tft.setCursor(16, 106);
  screen.tft.print("* High-Speed SRAM: 786 KB");
  screen.tft.setCursor(16, 118);
  screen.tft.print("* Operating RTOS: Zephyr RTOS");
  screen.tft.setCursor(16, 130);
  screen.tft.print("* Header Control: Shield Pin Master");

  // Card 2: Microprocessor (MPU Core)
  drawEInkCard(8, 166, SCREEN_W - 16, 105, "APPLICATION MPU (QUALCOMM QRB2210)");
  screen.tft.setCursor(16, 176);
  screen.tft.print("* CPU: Quad-Core Arm Cortex-A53");
  screen.tft.setCursor(16, 188);
  screen.tft.print("* Frequency: Up to 2.0 GHz");
  screen.tft.setCursor(16, 200);
  screen.tft.print("* System Memory: 4 GB LPDDR4 RAM");
  screen.tft.setCursor(16, 212);
  screen.tft.print("* Flash Storage: 16 GB eMMC 5.1");
  screen.tft.setCursor(16, 224);
  screen.tft.print("* Linux Host OS: Debian Linux");
  screen.tft.setCursor(16, 236);
  screen.tft.print("* Python / App Lab Multi-Client");
}

// ==============================================================================
// PAGE 2: WIRELESS & NETWORK (Wi-Fi, Bluetooth BLE 5.0, Bridge)
// ==============================================================================
void drawPage2_Wireless() {
  // Page Title Bar
  screen.tft.fillRect(8, 32, SCREEN_W - 16, 16, EINK_LIGHT_GRAY);
  screen.tft.drawRect(8, 32, SCREEN_W - 16, 16, EINK_BLACK);
  screen.tft.setTextColor(EINK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(14, 36);
  screen.tft.print("PAGE 2/3: WIRELESS & INTER-CORE");

  // Card 1: Dual-Band Wi-Fi Specs
  drawEInkCard(8, 60, SCREEN_W - 16, 80, "WI-FI CONNECTIVITY (QUALCOMM)");
  screen.tft.setCursor(16, 70);
  screen.tft.print("* Standards: 802.11 a/b/g/n/ac");
  screen.tft.setCursor(16, 82);
  screen.tft.print("* Frequencies: 2.4 GHz & 5.0 GHz");
  screen.tft.setCursor(16, 94);
  screen.tft.print("* Throughput: Up to 433 Mbps (1x1)");
  screen.tft.setCursor(16, 106);
  screen.tft.print("* Security: WPA2 / WPA3-Personal");
  screen.tft.setCursor(16, 118);
  screen.tft.print("* OTA Flashing: Supported (Port 192.x)");

  // Card 2: Bluetooth & BLE 5.0 Specs
  drawEInkCard(8, 150, SCREEN_W - 16, 68, "BLUETOOTH 5.0 & BLE");
  screen.tft.setCursor(16, 160);
  screen.tft.print("* Standard: Bluetooth Low Energy 5.0");
  screen.tft.setCursor(16, 172);
  screen.tft.print("* Data Rate: 2 Mbps LE 2M PHY");
  screen.tft.setCursor(16, 184);
  screen.tft.print("* Profiles: GATT Client / Server");
  screen.tft.setCursor(16, 196);
  screen.tft.print("* Backward Compatibility: BT 4.2 / Classic");

  // Card 3: Inter-Core Bridge Bus
  drawEInkCard(8, 227, SCREEN_W - 16, 45, "INTERNAL BUS BRIDGE");
  screen.tft.setCursor(16, 237);
  screen.tft.print("* Physical: High-Speed UART (/dev/ttyHS1)");
  screen.tft.setCursor(16, 249);
  screen.tft.print("* Protocol: MessagePack-RPC (arduino-router)");
}

// ==============================================================================
// PAGE 3: HARDWARE PINOUT & SHIELD DIAGNOSTICS
// ==============================================================================
void drawPage3_Pinout() {
  // Page Title Bar
  screen.tft.fillRect(8, 32, SCREEN_W - 16, 16, EINK_LIGHT_GRAY);
  screen.tft.drawRect(8, 32, SCREEN_W - 16, 16, EINK_BLACK);
  screen.tft.setTextColor(EINK_BLACK);
  screen.tft.setTextSize(1);
  screen.tft.setCursor(14, 36);
  screen.tft.print("PAGE 3/3: SHIELD INTERFACING & PINS");

  // Card 1: Adafruit 2.8" TFT Pin Mapping
  drawEInkCard(8, 60, SCREEN_W - 16, 115, "ADAFRUIT TFT SHIELD (PID 1947)");
  screen.tft.setCursor(16, 70);
  screen.tft.print("Signal     Uno Q Pin   STM32 GPIO");
  screen.tft.drawFastHLine(16, 79, SCREEN_W - 32, EINK_DARK_GRAY);
  screen.tft.setCursor(16, 84);
  screen.tft.print("TFT CS     D10         Digital Pin");
  screen.tft.setCursor(16, 96);
  screen.tft.print("TFT DC     D9          Digital Pin");
  screen.tft.setCursor(16, 108);
  screen.tft.print("SPI MOSI   D23 (ICSP)  STM32 PC3");
  screen.tft.setCursor(16, 120);
  screen.tft.print("SPI MISO   D22 (ICSP)  STM32 PC2");
  screen.tft.setCursor(16, 132);
  screen.tft.print("SPI SCK    D24 (ICSP)  STM32 PD1");
  screen.tft.setCursor(16, 144);
  screen.tft.print("Touch I2C  SDA / SCL   Wire (0x38)");

  // Card 2: Live Shield Sensor Telemetry
  drawEInkCard(8, 185, SCREEN_W - 16, 85, "LIVE PERIPHERAL TELEMETRY");
  screen.tft.setCursor(16, 195);
  screen.tft.print("* Display Driver: ILI9341 SPI (Online)");
  screen.tft.setCursor(16, 207);
  screen.tft.print("* Touch Controller: FT6206 Capacitive");
  screen.tft.setCursor(16, 219);
  screen.tft.print("* Active Orientation: Portrait (240x320)");
  screen.tft.setCursor(16, 231);
  screen.tft.print("* Calibration: Native 1:1 mapped");
  screen.tft.setCursor(16, 243);
  screen.tft.print("* Driver Lib: ArduinoUnoQ_AdafruitTouch");
}

void renderCurrentPage() {
  triggerEInkRefresh();
  drawEInkHeader();

  switch (currentPage) {
    case 0: drawPage1_Architecture(); break;
    case 1: drawPage2_Wireless();     break;
    case 2: drawPage3_Pinout();       break;
  }

  drawEInkNav();
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // Initialize in Portrait orientation (Rotation 0)
  screen.begin();
  screen.setRotation(0); // 240 x 320

  renderCurrentPage();
}

void loop() {
  if (screen.touched()) {
    TS_Point p = screen.getPoint();

    // Check Bottom Navigation Bar
    if (p.y >= NAV_Y && p.y <= NAV_Y + NAV_H) {
      // PREV Button
      if (p.x >= BTN_PREV_X && p.x <= BTN_PREV_X + BTN_PREV_W) {
        currentPage = (currentPage - 1 + TOTAL_PAGES) % TOTAL_PAGES;
        renderCurrentPage();
        delay(220);
        return;
      }
      // REFRESH Button
      else if (p.x >= BTN_REF_X && p.x <= BTN_REF_X + BTN_REF_W) {
        renderCurrentPage();
        delay(220);
        return;
      }
      // NEXT Button
      else if (p.x >= BTN_NEXT_X && p.x <= BTN_NEXT_X + BTN_NEXT_W) {
        currentPage = (currentPage + 1) % TOTAL_PAGES;
        renderCurrentPage();
        delay(220);
        return;
      }
    } else {
      // Tapping anywhere on page cycles to next page
      currentPage = (currentPage + 1) % TOTAL_PAGES;
      renderCurrentPage();
      delay(250);
    }
  }

  delay(20);
}
