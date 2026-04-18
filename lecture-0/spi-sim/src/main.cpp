#include <Adafruit_ILI9341.h>
#include <Arduino.h>
#include <SPI.h>

namespace {

// SPI wiring used by this project:
//   D10 -> TFT CS
//   D9  -> TFT D/C
//   D11 -> TFT MOSI
//   D13 -> TFT SCK
//   D7  -> Logic Analyzer trigger only
//   5V  -> TFT VCC
//   GND -> TFT GND
//
// MISO is intentionally not connected because this demo only writes to the
// display and never reads pixel data back from it.
constexpr uint8_t kTftCs = 10;
constexpr uint8_t kTftDc = 9;
constexpr uint8_t kCaptureTriggerPin = 7;

// The current goal is to make VCD export reliable, so we keep this enabled and
// send only a very small amount of SPI traffic during startup.
constexpr bool kCaptureFriendlyMode = true;

Adafruit_ILI9341 tft(kTftCs, kTftDc);

uint16_t rgb565(uint8_t red, uint8_t green, uint8_t blue) {
  return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
}

// Emit a short, finite SPI burst. The shapes are intentionally tiny so the
// logic analyzer only needs to capture a compact waveform window.
void runCaptureSequence() {
  tft.drawPixel(12, 12, ILI9341_RED);
  delay(40);

  tft.drawPixel(13, 12, ILI9341_GREEN);
  delay(40);

  tft.drawPixel(14, 12, ILI9341_BLUE);
  delay(40);

  tft.drawFastHLine(24, 28, 24, ILI9341_YELLOW);
  delay(40);

  tft.fillRect(24, 42, 12, 8, rgb565(180, 40, 220));
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(200);

  // This pin is connected only to the logic analyzer trigger input.
  // We keep it low during TFT initialization, then raise it only for the short
  // SPI burst we actually want to capture into the VCD file.
  pinMode(kCaptureTriggerPin, OUTPUT);
  digitalWrite(kCaptureTriggerPin, LOW);

  Serial.println();
  Serial.println(F("=== SPI TFT Demo ==="));
  Serial.println(F("board : Arduino Uno"));
  Serial.println(F("screen: ILI9341 TFT"));
  Serial.println(F("pins  : CS=D10 DC=D9 MOSI=D11 SCK=D13"));

  tft.begin();
  tft.setRotation(1);

  if (kCaptureFriendlyMode) {
    digitalWrite(kCaptureTriggerPin, HIGH);
    runCaptureSequence();
    digitalWrite(kCaptureTriggerPin, LOW);

    Serial.println(F("capture sequence finished"));
    Serial.println(F("logic analyzer trigger returned low"));
    Serial.println(F("loop() stays idle so stopping the simulator is easier"));
    return;
  }
}

void loop() {
  if (kCaptureFriendlyMode) {
    delay(250);
    return;
  }
}
