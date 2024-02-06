#include <WiFi.h>
#include <Arduino.h>

const char *ssid = "HANDY-M.O";
const char *password = "1234567890";

void setup()
{
  Serial.begin(9600);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
}

void loop()
{
  Serial.println(WiFi.localIP());
}