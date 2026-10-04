#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "RTClib.h"
#include <SPI.h>
#include <SD.h>

// ===== OLED =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===== RTC =====
RTC_DS3231 rtc;
bool rtcAvailable = false;

// ===== SD =====
#define SD_CS 5
bool sdAvailable = false;
String logFileName = "";

// ===== PINS =====
#define FLOW_PIN 27
#define RELAY_PIN 4

volatile int pulseCount = 0;
volatile unsigned long lastPulseTime = 0;

unsigned long lastMeasureTime = 0;
unsigned long startupTime = 0;
unsigned long lastLogTime = 0;

int measuredPulse = 0;

bool relayState = true;
int noFlowCount = 0;

#define STARTUP_DELAY 5000

// ================= ISR =================
void IRAM_ATTR countPulse() {
  unsigned long now = micros();
  if (now - lastPulseTime > 800) {
    pulseCount++;
    lastPulseTime = now;
  }
}

// ================= TIME =================
String getTime() {
  if (!rtcAvailable) return "NoRTC";

  DateTime now = rtc.now();
  char buffer[10];
  sprintf(buffer, "%02d:%02d:%02d",
          now.hour(), now.minute(), now.second());

  return String(buffer);
}

// ================= LOG FILE =================
void createLogFile() {
  if (!rtcAvailable) {
    logFileName = "/log.csv";
    return;
  }

  DateTime now = rtc.now();
  char filename[30];

  sprintf(filename, "/log_%02d%02d_%02d%02d.csv",
          now.day(), now.month(),
          now.hour(), now.minute());

  logFileName = String(filename);

  File file = SD.open(logFileName, FILE_WRITE);
  if (file) {
    file.println("TIME,PULSE,RELAY");
    file.close();
  }
}

// ================= LOG =================
void logData() {
  if (!sdAvailable) return;

  File file = SD.open(logFileName, FILE_APPEND);
  if (file) {
    file.print(getTime());
    file.print(",");
    file.print(measuredPulse);
    file.print(",");
    file.println(relayState ? "ON" : "OFF");
    file.close();
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(FLOW_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(FLOW_PIN), countPulse, RISING);

  digitalWrite(RELAY_PIN, LOW); // ON (active LOW)

  // I2C
  Wire.begin(21, 22);

  // OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(1);
  display.setTextColor(WHITE);

  // RTC
  if (rtc.begin()) rtcAvailable = true;

  // SD
  if (SD.begin(SD_CS)) {
    sdAvailable = true;
    createLogFile();
  }

  startupTime = millis();
}

// ================= LOOP =================
void loop() {

  if (millis() - lastMeasureTime >= 1000) {

    measuredPulse = pulseCount;
    pulseCount = 0;
    lastMeasureTime = millis();

    bool isStartup = false;

    // ===== STARTUP =====
    if (millis() - startupTime < STARTUP_DELAY) {
      relayState = true;
      digitalWrite(RELAY_PIN, LOW);
      isStartup = true;
    } 
    else {

      // ===== YOUR ORIGINAL LOGIC (UNCHANGED) =====
      bool validFlow = (measuredPulse > 100 && measuredPulse < 500);
      bool noFlow = (measuredPulse <= 20);
      bool abnormalHigh = (measuredPulse >= 500);

      if (validFlow) {
        relayState = true;
        noFlowCount = 0;
      }

      if (noFlow || abnormalHigh) {
        noFlowCount++;
      } else {
        noFlowCount = 0;
      }

      if (noFlowCount >= 2) {
        relayState = false;
      }

      digitalWrite(RELAY_PIN, relayState ? LOW : HIGH);
    }

    // ===== DISPLAY =====
    display.clearDisplay();

    display.setCursor(0, 0);
    display.println("PUMP SYSTEM");

    display.setCursor(0, 10);
    display.print("Time: ");
    display.println(getTime());

    display.setCursor(0, 20);
    display.print("Relay: ");
    display.println(relayState ? "ON" : "OFF");

    display.setCursor(0, 30);
    display.print("Flow: ");
    if (measuredPulse > 100 && measuredPulse < 500)
      display.println("OK");
    else
      display.println("NO");

    display.setCursor(0, 40);
    display.print("State: ");
    if (isStartup)
      display.println("START");
    else if (relayState)
      display.println("NORMAL");
    else
      display.println("FAULT");

    display.display();

    // ===== SERIAL =====
    Serial.print("Pulse: ");
    Serial.print(measuredPulse);
    Serial.print(" | Relay: ");
    Serial.println(relayState ? "ON" : "OFF");

    // ===== LOG EVERY 5 SEC =====
    if (millis() - lastLogTime > 5000) {
      logData();
      lastLogTime = millis();
    }
  }
}
