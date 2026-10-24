#include <Wire.h>
#include <U8g2lib.h>

// === Display (most 1.3" I2C OLEDs are SH1106) ===
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
// If SSD1306, use this instead:
// U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

const long ADC_REFERENCE_MV = 5000;
const long ADC_MAX_VALUE = 1023;
// Fit a 1M resistor from A0 to GND so an unplugged sensor input reads near zero.
const int SENSOR_DISCONNECTED_BELOW_MV = 50;
const uint8_t ADC_AVERAGE_SAMPLES = 8;
const float VOLTAGE_SMOOTHING_ALPHA = 0.50f;
const uint8_t GRAPH_WIDTH = 124;
const uint8_t GRAPH_HEIGHT = 38;
const uint8_t GRAPH_X = 2;
const uint8_t GRAPH_Y = 2;
const uint8_t LED1_PIN = 13;
const uint8_t LED2_PIN = 6;
const unsigned long SAMPLE_INTERVAL_MS = 160;
const unsigned long LED1_TOGGLE_MS = 500;

uint16_t voltageHistory[GRAPH_WIDTH];
uint8_t historyIndex = 0;
uint8_t historyCount = 0;
unsigned long lastSampleMs = 0;
unsigned long lastLed1ToggleMs = 0;
unsigned long lastLed2ToggleMs = 0;
unsigned long led2IntervalMs = 0;
bool led1State = false;
bool led2State = false;
float smoothedVoltageMv = 0.0f;
bool voltageFilterInitialized = false;

int readAveragedVoltageMillivolts() {
  uint32_t rawTotal = 0;
  for (uint8_t i = 0; i < ADC_AVERAGE_SAMPLES; i++) {
    rawTotal += analogRead(A0);
  }
  const uint16_t averageRaw = (rawTotal + ADC_AVERAGE_SAMPLES / 2) / ADC_AVERAGE_SAMPLES;
  return (int)(((long)averageRaw * ADC_REFERENCE_MV + ADC_MAX_VALUE / 2) / ADC_MAX_VALUE);
}

int smoothVoltageMillivolts(int measuredMv) {
  if (measuredMv < SENSOR_DISCONNECTED_BELOW_MV) {
    smoothedVoltageMv = 0.0f;
    voltageFilterInitialized = false;
    return 0;
  }

  if (!voltageFilterInitialized) {
    smoothedVoltageMv = measuredMv;
    voltageFilterInitialized = true;
  } else {
    smoothedVoltageMv += VOLTAGE_SMOOTHING_ALPHA * (measuredMv - smoothedVoltageMv);
  }
  return (int)(smoothedVoltageMv + 0.5f);
}

// --- Draw whole panel ---
void drawPanel(int voltageMv) {
  uint16_t minimum = voltageHistory[0];
  uint16_t maximum = voltageHistory[0];
  for (uint8_t i = 1; i < historyCount; i++) {
    if (voltageHistory[i] < minimum) minimum = voltageHistory[i];
    if (voltageHistory[i] > maximum) maximum = voltageHistory[i];
  }

  uint16_t range = maximum - minimum;
  if (range < 10) range = 10;
  uint16_t padding = range / 10;
  int scaleMinimum = (int)minimum - padding;
  int scaleMaximum = (int)maximum + padding;
  if (scaleMaximum == scaleMinimum) scaleMaximum++;

  char voltageLine[16];
  snprintf(voltageLine, sizeof(voltageLine), "%d mV", voltageMv);

  u8g2.firstPage();
  do {
    u8g2.drawFrame(GRAPH_X, GRAPH_Y, GRAPH_WIDTH, GRAPH_HEIGHT);
    for (uint8_t x = 1; x < historyCount; x++) {
      uint8_t previousIndex = (historyIndex + GRAPH_WIDTH - historyCount + x - 1) % GRAPH_WIDTH;
      uint8_t currentIndex = (previousIndex + 1) % GRAPH_WIDTH;
      long previousOffset = ((long)voltageHistory[previousIndex] - scaleMinimum) *
        (GRAPH_HEIGHT - 4) / (scaleMaximum - scaleMinimum);
      long currentOffset = ((long)voltageHistory[currentIndex] - scaleMinimum) *
        (GRAPH_HEIGHT - 4) / (scaleMaximum - scaleMinimum);
      int previousY = GRAPH_Y + GRAPH_HEIGHT - 2 - previousOffset;
      int currentY = GRAPH_Y + GRAPH_HEIGHT - 2 - currentOffset;
      u8g2.drawLine(GRAPH_X + x, previousY, GRAPH_X + x + 1, currentY);
    }

    u8g2.setFont(u8g2_font_4x6_tr);
    u8g2.drawStr(2, 49, "Microbial fuel cell voltage");
    u8g2.setFont(u8g2_font_6x12_tr);
    u8g2.drawStr(2, 63, voltageLine);
  } while (u8g2.nextPage());
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(100000);
  u8g2.begin();
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  randomSeed(analogRead(A0) ^ micros());
  led2IntervalMs = random(150, 1200);
}

void loop() {
  unsigned long now = millis();

  if (now - lastLed1ToggleMs >= LED1_TOGGLE_MS) {
    lastLed1ToggleMs = now;
    led1State = !led1State;
    digitalWrite(LED1_PIN, led1State ? HIGH : LOW);
  }

  if (now - lastLed2ToggleMs >= led2IntervalMs) {
    lastLed2ToggleMs = now;
    led2State = !led2State;
    digitalWrite(LED2_PIN, led2State ? HIGH : LOW);
    led2IntervalMs = random(150, 1200);
  }

  if (now - lastSampleMs < SAMPLE_INTERVAL_MS) return;
  lastSampleMs = now;

  const int measuredMv = readAveragedVoltageMillivolts();
  const int voltageMv = smoothVoltageMillivolts(measuredMv);
  voltageHistory[historyIndex] = voltageMv;
  historyIndex = (historyIndex + 1) % GRAPH_WIDTH;
  if (historyCount < GRAPH_WIDTH) historyCount++;

  drawPanel(voltageMv);
  Serial.print(voltageMv);
  Serial.println(" mV");
}