#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Arduino.h>
#include <SPI.h>

namespace {

// This demo uses a single Arduino Uno as the SPI master.
// The Uno continuously sends drawing commands and pixel data to an ILI9341 TFT.
//
// Wiring used by this project:
//   D10 -> TFT CS
//   D9  -> TFT D/C
//   D11 -> TFT MOSI
//   D13 -> TFT SCK
//   5V  -> TFT VCC
//   GND -> TFT GND
//
// MISO is intentionally left disconnected because this demo only writes to the
// display and never reads data back from it.

// Dedicated control pins for the display. The SPI data and clock pins are the
// hardware SPI pins of the ATmega328P and are handled by the SPI library.
constexpr uint8_t kTftCs = 10;
constexpr uint8_t kTftDc = 9;

// The screen is rotated to landscape mode in setup(), so the logical drawing
// area becomes 320x240.
constexpr uint16_t kScreenWidth = 320;
constexpr uint16_t kScreenHeight = 240;

// A small set of colors used by the custom UI.
constexpr uint16_t kBackground = 0x1104;
constexpr uint16_t kPanel = 0x2128;
constexpr uint16_t kAccent = 0x041F;

// Adafruit_ILI9341 wraps the low-level SPI protocol required by the display.
// Once this object is initialized, drawing APIs such as fillRect() and print()
// will be translated into SPI transactions automatically.
Adafruit_ILI9341 tft(kTftCs, kTftDc);

// Convert 8-bit per channel RGB values into the 16-bit RGB565 format used by
// the ILI9341 controller.
uint16_t rgb565(uint8_t red, uint8_t green, uint8_t blue) {
  return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
}

// Generate a smoothly changing color. The animation code uses this helper to
// keep the progress bar and accent line visually "alive" without storing any
// lookup table in memory.
uint16_t colorWheel(uint8_t value) {
  if (value < 85) {
    return rgb565(value * 3, 255 - value * 3, 40);
  }
  if (value < 170) {
    value -= 85;
    return rgb565(255 - value * 3, 40, value * 3);
  }
  value -= 170;
  return rgb565(40, value * 3, 255 - value * 3);
}

// Draw the top banner once during initialization. This is static UI, so there
// is no reason to redraw it in every animation frame.
void drawHeader() {
  tft.fillRect(0, 0, kScreenWidth, 34, kAccent);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(12, 8);
  tft.print(F("ChenlongOS SPI Demo"));
}

// Draw the left information panel. The goal is to make the wiring visible on
// the TFT itself, so the screen can also serve as a quick reference when the
// simulation is running.
void drawWiringPanel() {
  tft.fillRoundRect(10, 46, 138, 184, 10, kPanel);
  tft.drawRoundRect(10, 46, 138, 184, 10, ILI9341_CYAN);

  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(2);
  tft.setCursor(24, 60);
  tft.print(F("Wiring"));

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);

  tft.setCursor(20, 92);
  tft.print(F("CS   -> D10"));
  tft.setCursor(20, 110);
  tft.print(F("DC   -> D9"));
  tft.setCursor(20, 128);
  tft.print(F("MOSI -> D11"));
  tft.setCursor(20, 146);
  tft.print(F("SCK  -> D13"));
  tft.setCursor(20, 164);
  tft.print(F("MISO -> unused"));

  tft.setTextColor(ILI9341_GREEN);
  tft.setCursor(20, 194);
  tft.print(F("SPI writes pixels"));
  tft.setCursor(20, 208);
  tft.print(F("to the TFT controller"));
}

// Draw the right display panel. Most of the shapes are static placeholders that
// define where the animated data will be refreshed later.
void drawDemoPanel() {
  tft.fillRoundRect(162, 46, 148, 184, 10, kPanel);
  tft.drawRoundRect(162, 46, 148, 184, 10, ILI9341_MAGENTA);

  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(2);
  tft.setCursor(182, 60);
  tft.print(F("Display"));

  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(176, 90);
  tft.print(F("progress"));
  tft.drawRect(176, 104, 120, 18, ILI9341_WHITE);

  tft.setCursor(176, 138);
  tft.print(F("color sweep"));
  for (uint8_t i = 0; i < 6; ++i) {
    tft.fillRect(176 + i * 20, 152, 18, 18, colorWheel(i * 42));
  }

  tft.setCursor(176, 184);
  tft.print(F("frame:"));
  tft.setCursor(176, 206);
  tft.print(F("uptime:"));
}

// Compose the full static UI. Calling this once is much cheaper than redrawing
// the whole screen every frame on an 8-bit microcontroller.
void drawStaticUi() {
  tft.fillScreen(kBackground);
  drawHeader();
  drawWiringPanel();
  drawDemoPanel();
}

// Update only the inside of the progress bar. First clear the old fill area,
// then paint the new value using the current animation color.
void updateProgressBar(uint8_t percent, uint16_t color) {
  const uint16_t innerWidth = 116;
  const uint16_t filled = (static_cast<uint32_t>(innerWidth) * percent) / 100;
  tft.fillRect(178, 106, innerWidth, 14, ILI9341_BLACK);
  tft.fillRect(178, 106, filled, 14, color);
}

// Refresh the numeric values shown on the right panel. Each field is cleared in
// a small local rectangle before printing the new text, which avoids flickering
// and keeps the redraw cost predictable.
void updateStatus(uint16_t frame, uint32_t uptimeMs, uint16_t color) {
  char buffer[20];

  tft.fillRect(220, 180, 76, 14, kPanel);
  tft.setTextColor(color);
  tft.setCursor(220, 184);
  snprintf(buffer, sizeof(buffer), "%u", frame);
  tft.print(buffer);

  tft.fillRect(220, 202, 76, 14, kPanel);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(220, 206);
  snprintf(buffer, sizeof(buffer), "%lus", uptimeMs / 1000UL);
  tft.print(buffer);
}

// Produce one animation frame. The progress percentage loops from 0 to 100,
// while the color is derived from the frame counter so the UI keeps changing.
//
// Only the parts that actually move are updated:
//   - the progress bar fill
//   - the frame counter
//   - the uptime text
//   - the small accent line near the bottom
void updateAnimation(uint16_t frame) {
  const uint8_t percent = frame % 101;
  const uint16_t color = colorWheel((frame * 5) & 0xFF);
  const int16_t x = 176 + ((static_cast<uint32_t>(116) * percent) / 100);

  updateProgressBar(percent, color);
  updateStatus(frame, millis(), color);

  tft.fillRect(176, 176, 120, 4, kPanel);
  tft.fillRect(176, 176, x - 176, 4, color);
}

}  // namespace

void setup() {
  // Serial output is only used as a human-readable trace so that the terminal
  // and the TFT tell the same story during the demo.
  Serial.begin(115200);
  delay(200);

  Serial.println();
  Serial.println(F("=== SPI TFT Demo ==="));
  Serial.println(F("board : Arduino Uno"));
  Serial.println(F("screen: ILI9341 TFT"));
  Serial.println(F("pins  : CS=D10 DC=D9 MOSI=D11 SCK=D13"));

  // Initialize the TFT controller and switch to landscape orientation.
  // After begin(), all drawing commands below will be sent through SPI.
  tft.begin();
  tft.setRotation(1);

  // Draw the static background once, then paint the initial dynamic state.
  drawStaticUi();
  updateAnimation(0);

  Serial.println(F("display initialized"));
}

void loop() {
  // Keep frame state between iterations. On Arduino, loop() runs forever, so
  // static locals are a simple way to maintain animation state.
  static uint16_t frame = 0;
  static uint32_t lastUpdateAt = 0;

  const uint32_t now = millis();

  // Limit the refresh rate to roughly 12.5 FPS. This keeps the animation smooth
  // enough for the demo without redrawing unnecessarily fast on an Uno.
  if (now - lastUpdateAt < 80) {
    return;
  }

  lastUpdateAt = now;
  frame++;

  // Send the next incremental UI update to the TFT over SPI.
  updateAnimation(frame);
}
