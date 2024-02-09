#include <WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

void IRAM_ATTR timerISR();
void callback(char *topic, byte *payload, unsigned int length);
void reconnect();
void noMessages();
void systemPartlyDown();

const char *ssid = "";
const char *password = "";
const char *mqtt_server = "";
const char *topic = "raspi-01/health";

volatile int sec = 0;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
hw_timer_t *timer = NULL;
WiFiClient espClient;
PubSubClient client(espClient);

void setupInterrupt()
{
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &timerISR, true);
  timerAlarmWrite(timer, 1000000, true);
  timerAlarmEnable(timer);

  Serial.println("Timer interrupt setup complete.");
}

void setup()
{
  setCpuFrequencyMhz(80);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  setupInterrupt();
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
}

// Interrupt Service Routine for the timer
void IRAM_ATTR timerISR()
{
  portENTER_CRITICAL_ISR(&timerMux);
  sec++;
  if (sec > 180)
  {
    noMessages();
    sec = 0;
  }

  portEXIT_CRITICAL_ISR(&timerMux);
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.println("Message arrived in topic: " + String(topic));

  // Convert payload to a string
  String payloadStr;
  for (int i = 0; i < length; i++)
  {
    payloadStr += (char)payload[i];
  }

  // Parse JSON
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, payloadStr);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  // Access JSON data
  const char *value = doc["timestamp"];
  Serial.print("Received value: ");
  Serial.println(value);
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client"))
    {
      Serial.println("connected");
      client.subscribe(topic);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void noMessages()
{
  Serial.println("No MQTT messages recieved for some time");
}

void systemPartlyDown()
{
  Serial.println("The system is partly down");
}