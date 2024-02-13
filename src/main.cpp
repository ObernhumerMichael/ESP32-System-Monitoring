#include <WiFi.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP_Mail_Client.h>

void reconnectWIFI();
void callback(char *topic, byte *payload, unsigned int length);
void reconnectMQTT();
void IRAM_ATTR timerISR();
void checkMessage(DynamicJsonDocument doc);
void noMessages();
void systemPartlyDown(DynamicJsonDocument doc);
SMTP_Message createSMTPMessage(String textMsg);
void sendSMTPMessage(SMTP_Message message);
void smtpCallback(SMTP_Status status);

// For the WiFi connection
WiFiClient espClient;
#define WIFI_SSID "HANDY-M.O"
#define WIFI_PASSWORD "1234567890"

// For the MQTT client
const char *mqtt_server = "192.168.196.141";
const char *topic = "raspi-01/health";
PubSubClient client(espClient);

// For the email functionality
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
#define AUTHOR_EMAIL "shadowrezkin@gmail.com"
#define AUTHOR_PASSWORD "ccfjrecezsoocdge"
#define RECIPIENT_EMAIL "michaelobernhumer@gmail.com"
SMTPSession smtp;
Session_Config config; /*for user defined session credentials */

//  Timer Interrupt
volatile int sec = 0;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
hw_timer_t *timer = NULL;

// general
volatile bool isClientSystemAlive = true;

// Setup functions

void setupWiFi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
}

void setupMQTT()
{
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setupInterrupt()
{
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &timerISR, true);
  timerAlarmWrite(timer, 1000000, true);
  timerAlarmEnable(timer);

  Serial.println("Timer interrupt setup complete.");
}

void setupSMTP()
{
  /*  Set the network reconnection option */
  MailClient.networkReconnect(true);

  /** Enable the debug via Serial port
   * 0 for no debugging
   * 1 for basic level debugging
   *
   * Debug port can be changed via ESP_MAIL_DEFAULT_DEBUG_PORT in ESP_Mail_FS.h
   */
  smtp.debug(1);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

  /* Set the session config */
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;
  config.login.user_domain = "";

  /*
  Set the NTP config time
  For times east of the Prime Meridian use 0-12
  For times west of the Prime Meridian add 12 to the offset.
  Ex. American/Denver GMT would be -6. 6 + 12 = 18
  See https://en.wikipedia.org/wiki/Time_zone for a list of the GMT/UTC timezone offsets
  */
  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = 1;
  config.time.day_light_offset = 0;

  SMTP_Message message = createSMTPMessage("This is a test message when the esp was initialized.");
  sendSMTPMessage(message);
}

void setup()
{
  setCpuFrequencyMhz(80);
  Serial.begin(115200);
  setupWiFi();
  setupMQTT();
  setupInterrupt();
  setupSMTP();
}

void loop()
{
  reconnectWIFI();
  reconnectMQTT();
  client.loop();
}

// reconnects the esp to the WiFi if it lost its connection
void reconnectWIFI()
{
  // Check if WiFi connection is still active
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi connection lost. Reconnecting...");

    WiFi.reconnect();

    // Wait for connection to be established
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(1000);
      Serial.print(".");
    }

    Serial.println("WiFi reconnected!");
  }
}

// callback for an mqtt message
void callback(char *topic, byte *payload, unsigned int length)
{
  // reset client system health vars
  sec = 0;
  isClientSystemAlive = true;

  Serial.print("Message arrived in topic: ");
  Serial.println(String(topic));

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

  checkMessage(doc);
}

// reconnects the esp to the mqtt broker if it lost its connection
void reconnectMQTT()
{
  if (!client.connected())
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
        Serial.println("try again in 5 seconds");
        delay(5000);
      }
    }
  }
}

// checks the mqtt message if all the systems are healthy
void checkMessage(DynamicJsonDocument doc)
{
  Serial.println("Checking the MQTT message");

  const char *timestamp = doc["timestamp"];

  bool isDNSFunctional = false;
  bool isInternetReachable = false;
  bool isFileSystemAlive = false;

  if (doc.containsKey("isDNSFunctional"))
  {
    isDNSFunctional = doc["isDNSFunctional"].as<bool>();
  }
  if (doc.containsKey("isInternetReachable"))
  {
    isInternetReachable = doc["isInternetReachable"].as<bool>();
  }
  if (doc.containsKey("isFileSystemAlive"))
  {
    isFileSystemAlive = doc["isFileSystemAlive"].as<bool>();
  }

  if (isDNSFunctional && isInternetReachable && isFileSystemAlive)
  {
    Serial.println("Everything is up and running!");
  }
  else
  {
    systemPartlyDown(doc);
  }
}

// Interrupt Service Routine for the timer
void IRAM_ATTR timerISR()
{
  portENTER_CRITICAL_ISR(&timerMux);
  if (isClientSystemAlive)
  {
    sec++;
    Serial.println(sec);
    if (sec > 40)
    {
      sec = 0;
      isClientSystemAlive = false;
      noMessages();
    }
  }
  portEXIT_CRITICAL_ISR(&timerMux);
}

// is called when the esp doesn't receive any heartbeats anymore
void noMessages()
{
  Serial.println("No MQTT messages received for some time");
  SMTP_Message message = createSMTPMessage("There are no longer any heartbeats received");
  sendSMTPMessage(message);
}

// is called when a part of a client system is down but still send heartbeats
void systemPartlyDown(DynamicJsonDocument healthStatusRaw)
{
  Serial.println("The system is partly down");

  // Serialize the JSON document to a String
  String healthStatus;
  serializeJsonPretty(healthStatusRaw, healthStatus);

  // Convert the const char[] to a String object
  String text = "The system sends heartbeats it failed some internal checks!:\n" + healthStatus;

  SMTP_Message message = createSMTPMessage(text);
  sendSMTPMessage(message);
}

/* Creates a SMTP message body */
SMTP_Message createSMTPMessage(String textMsg)
{
  /* Declare the message class */
  SMTP_Message message;

  /* Set the message headers */
  message.sender.name = F("ESP");
  message.sender.email = AUTHOR_EMAIL;
  message.subject = F("SYSTEM HEALTH WARNING!!!");
  message.addRecipient(F("Admin"), RECIPIENT_EMAIL);

  /*Send HTML message*/
  /*String htmlMsg = "<div style=\"color:#2f4468;\"><h1>Hello World!</h1><p>- Sent from ESP board</p></div>";
  message.html.content = htmlMsg.c_str();
  message.html.content = htmlMsg.c_str();
  message.text.charSet = "us-ascii";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;*/

  // Send raw text message
  message.text.content = textMsg.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  return message;
}

/* Sends a SMTP message according to the initial configurations */
void sendSMTPMessage(SMTP_Message message)
{
  if (!smtp.connect(&config))
  {
    ESP_MAIL_PRINTF("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    return;
  }

  if (!smtp.isLoggedIn())
  {
    Serial.println("\nNot yet logged in.");
  }
  else
  {
    if (smtp.isAuthenticated())
      Serial.println("\nSuccessfully logged in.");
    else
      Serial.println("\nConnected with no Auth.");
  }

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message))
    ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
}

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status)
{
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success())
  {
    // ESP_MAIL_PRINTF used in the examples is for format printing via debug Serial port
    // that works for all supported Arduino platform SDKs e.g. AVR, SAMD, ESP32 and ESP8266.
    // In ESP8266 and ESP32, you can use Serial.printf directly.

    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);

      // In case, ESP32, ESP8266 and SAMD device, the timestamp get from result.timestamp should be valid if
      // your device time was synched with NTP server.
      // Other devices may show invalid timestamp as the device time was not set i.e. it will show Jan 1, 1970.
      // You can call smtp.setSystemTime(xxx) to set device time manually. Where xxx is timestamp (seconds since Jan 1, 1970)

      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

    // You need to clear sending result as the memory usage will grow up.
    smtp.sendingResult.clear();
  }
}