#define BLYNK_TEMPLATE_ID "TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "CamBienThongMinhMAX30100"
#define BLYNK_AUTH_TOKEN "Token_Blynk"

#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiClientSecure.h>

char ssid[] = "wifi_name";
char pass[] = "wifi_pass";


const char* host = "script.google.com";
const int httpsPort = 443;
String GAS_ID = "ID";
WiFiClientSecure client;

Adafruit_SSD1306 display(128, 64, &Wire, -1);

#define REPORTING_PERIOD_MS 1000
#define GOOGLE_SHEET_PERIOD_MS 10000  // Gửi lên Google Sheets mỗi 10 giây

PulseOximeter pox;
uint32_t tsLastReport = 0;
uint32_t tsLastGoogleSheet = 0;
float lastValidHR = 0;
float lastValidSpO2 = 0;

void onBeatDetected() {
  Serial.println("Beat!");
}

void setup() {
  Serial.begin(115200);

  // Connect to WiFi and Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  // Setup for Google Sheets HTTPS connection
  client.setInsecure();

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Initializing...");
  display.display();

  // Initialize MAX30100
  if (!pox.begin()) {
    Serial.println("MAX30100 FAILED");
    display.setCursor(0, 10);
    display.println("MAX30100 FAILED");
    display.display();
    while (true);
  } else {
    Serial.println("MAX30100 SUCCESS");
    display.setCursor(0, 10);
    display.println("MAX30100 SUCCESS");
    display.display();
  }

  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop() {
  Blynk.run();
  pox.update();

  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    float hr = pox.getHeartRate();
    float spo2 = pox.getSpO2();

    Serial.print("Heart rate: ");
    Serial.print(hr);
    Serial.print(" bpm / SpO2: ");
    Serial.print(spo2);
    Serial.println(" %");

    // Send to Blynk Cloud
    Blynk.virtualWrite(V0, hr);
    Blynk.virtualWrite(V1, spo2);

    // OLED Display
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("Pulse Oximeter");
    display.setTextSize(2);
    display.setCursor(0, 20);
    display.print("HR: ");
    display.print((int)hr);
    display.println(" bpm");
    display.setCursor(0, 45);
    display.print("SpO2: ");
    display.print((int)spo2);
    display.println(" %");
    display.display();

    tsLastReport = millis();
  }

  // Gửi dữ liệu lên Google Sheets mỗi 10 giây (chỉ gửi khi có dữ liệu hợp lệ)
  if (millis() - tsLastGoogleSheet > GOOGLE_SHEET_PERIOD_MS) {
    float hr = pox.getHeartRate();
    float spo2 = pox.getSpO2();
    
    // Chỉ gửi khi có dữ liệu hợp lệ (hr > 0 và spo2 > 0)
    if (hr > 0 && spo2 > 0) {
      lastValidHR = hr;
      lastValidSpO2 = spo2;
      upDataToGoogleSheet(lastValidHR, lastValidSpO2);
    }
    tsLastGoogleSheet = millis();
  }
}


void upDataToGoogleSheet(float hr, float spo2) {
  Serial.print("Connecting to Google Sheets... ");
  
  if (!client.connect(host, httpsPort)) {
    Serial.println("Connection failed!");
    return;
  }
  

  String Send_Data_URL = "sts=write";
  Send_Data_URL += "&hr=" + String(hr, 1);
  Send_Data_URL += "&spo2=" + String(spo2, 1);

  String url = "/macros/s/" + GAS_ID + "/exec?" + Send_Data_URL;
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: ESP8266_MAX30100\r\n" +
               "Connection: close\r\n\r\n");
  
  Serial.println("Data sent to Google Sheets!");
}
