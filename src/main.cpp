#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>

#define MOSFET 4
#define BUTTON 12
#define LED 13

unsigned long buttondelay = 300;
unsigned long buttonlast = 0;
unsigned int led_brightness;

void wifiConnect()
{
  WiFiManager wifiManager;
  wifiManager.setTimeout(180);
  if (!wifiManager.autoConnect("PrinterLEDAP"))
  {
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }
}

void otaSetup()
{
  ArduinoOTA.setPassword("PrinterLEDDimmer");
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
    {
      type = "sketch";
    }
    else
    { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
    {
      Serial.println("Auth Failed");
    }
    else if (error == OTA_BEGIN_ERROR)
    {
      Serial.println("Begin Failed");
    }
    else if (error == OTA_CONNECT_ERROR)
    {
      Serial.println("Connect Failed");
    }
    else if (error == OTA_RECEIVE_ERROR)
    {
      Serial.println("Receive Failed");
    }
    else if (error == OTA_END_ERROR)
    {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

void setup()
{
  //  Serial.begin(115200);
  EEPROM.begin(512);
  wifiConnect();
  otaSetup();
  pinMode(LED, OUTPUT);
  pinMode(MOSFET, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  EEPROM.get(1, led_brightness);
  Serial.println(led_brightness);
  analogWriteRange(255);
  analogWriteFreq(200);
  analogWrite(MOSFET, led_brightness);
}

void loop()
{
  ArduinoOTA.handle();
  if (digitalRead(BUTTON) == LOW)
  {
    if (millis() - buttonlast > buttondelay || buttonlast == 0)
    {
      Serial.println("Button Pressed");
      switch (led_brightness)
      {
      case 255:
        Serial.println("100%->50%");
        led_brightness = 128;
        Serial.println(led_brightness);
        break;
      case 128:
        Serial.println("50%->10%");
        led_brightness = 32;
        Serial.println(led_brightness);
        break;
      case 32:
        Serial.println("12%->7%");
        led_brightness = 8;
        Serial.println(led_brightness);
        break;
      case 8:
        Serial.println("3%->1%");
        led_brightness = 1;
        Serial.println(led_brightness);
        break;
      case 1:
        Serial.println("3%->0%");
        led_brightness = 0;
        Serial.println(led_brightness);
        break;
      case 0:
        Serial.println("0%->100%");
        led_brightness = 255;
        Serial.println(led_brightness);
        break;
      default:
        Serial.println(led_brightness);
        Serial.println("Halvany fingom sincs mi tortent, 100%-ra allitok");
        led_brightness = 255;
      }
      EEPROM.put(1, led_brightness);
      EEPROM.commit();
      analogWrite(MOSFET, led_brightness);
      buttonlast = millis();
    }
  }
}
