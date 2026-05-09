/*************************************************
 * Project: Real-Time Electricity Consumption Monitor
 * Creator: Jai
 * Board  : ESP32
 * Sensor : ACS712
 *************************************************/

#include <WiFi.h>
#include <ThingSpeak.h>

const char* ssid = "OnePlus NordCE 5G";
const char* password = "88888888";

unsigned long channelID = 3233494;
const char* writeAPIKey = "4EVSORJ2WHXF9SVK";
WiFiClient client;

#define CURRENT_SENSOR_PIN 34
#define ADC_RESOLUTION 4095.0
#define VREF 3.3

float sensitivity = 0.100;
float voltageAC = 230.0;

float getSampledCurrent() {
  float voltageSum = 0;
  uint32_t samples = 0;
  unsigned long startTime = millis();
  
  while (millis() - startTime < 100) {
    float v = (analogRead(CURRENT_SENSOR_PIN) * VREF) / ADC_RESOLUTION;
    float i = (v - 2.5) / sensitivity;
    voltageSum += (i * i);
    samples++;
  }
  
  float rmsCurrent = sqrt(voltageSum / samples);
  if (rmsCurrent < 0.12) rmsCurrent = 0;
  return rmsCurrent;
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  ThingSpeak.begin(client);
}

void loop() {
  float current = getSampledCurrent();
  float powerKW = (voltageAC * current) / 1000.0;
  
  Serial.print("A: "); Serial.println(current);
  Serial.print("V: "); Serial.println(voltageAC);
  Serial.print("kW: "); Serial.println(powerKW);

  ThingSpeak.setField(1, powerKW);
  ThingSpeak.setField(2, voltageAC);
  ThingSpeak.setField(3, current);

  int response = ThingSpeak.writeFields(channelID, writeAPIKey);
  
  if (response == 200) {
    Serial.println("Updated");
  } else {
    Serial.println("Error: " + String(response));
  }

  delay(15000);
}