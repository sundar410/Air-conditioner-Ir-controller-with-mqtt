#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>                   // MQTT
#include <DaikinHeatpumpIR.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 
#include "Timer.h"

#define TOPSZ                  60           // Max number of characters in topic string
#define MESSZ                  240          // Max number of characters in JSON message string
#define IR_SEND_PIN 12

#define MQTT_HOST "postman.cloudmqtt.com"
#define MQTT_PORT 18822
#define MQTT_USER "username"
#define MQTT_PASS "password"
#define MQTT_CLIENT_ID "daikinremote"
#define MQTT_STATUS_CHANNEL "stat/daikin"
#define MQTT_COMMAND_CHANNEL "cmnd/daikin/#" // dont miss the # at the end!


WiFiClient espClient;               // Wifi Client
PubSubClient mqttClient(espClient);   // MQTT Client

IRSenderBitBang irSender(IR_SEND_PIN);

DaikinHeatpumpIR *heatpump;

Timer t;

// Set defaults
uint8_t AC_POWER = POWER_OFF,
        AC_MODE = MODE_AUTO,
        AC_FAN = FAN_AUTO,
        AC_TEMP = 24,
        AC_VSWING = VDIR_AUTO,
        AC_HSWING = HDIR_AUTO;

void setup() {
  Serial.begin(115200);
      //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    //reset saved settings
    //wifiManager.resetSettings();
    
    //set custom ip for portal
    wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

    //fetches ssid and pass from eeprom and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration
   Serial.printf("ESP8266 Chip id=%08X\n",ESP.getChipId());
   String ssid = "AIRPOWER_" + String(ESP.getChipId());
   wifiManager.autoConnect(ssid.c_str(), NULL);
    //wifiManager.autoConnect("AutoConnectAP");
    //or use this for auto generated name ESP + ChipID
    //wifiManager.autoConnect();

    
    //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  delay(10);
  Serial.println("Booting");

//  initWIFI();
  initOTA();

 /* Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());*/

  // Init loop timers
  t.every(5000, fiveLoop);
  t.every(10000, tenLoop);

  heatpump = new DaikinHeatpumpIR();

  initMQTT();
}

void loop() {
  ArduinoOTA.handle();
  mqttClient.loop();
  t.update();
}


void tenLoop() {
  Serial.println("Ping...");
  publishState();
}

// Reconnect to mqtt every 5 seconds if connection is lost
void fiveLoop() {
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
}

void initMQTT() {
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  publishState();
}

void mqttDataCb(char* topic, byte* data, unsigned int data_len) {
  char svalue[MESSZ];
  char topicBuf[TOPSZ];
  char dataBuf[data_len+1];

  strncpy(topicBuf, topic, sizeof(topicBuf));
  memcpy(dataBuf, data, sizeof(dataBuf));
  dataBuf[sizeof(dataBuf)-1] = 0;

  snprintf_P(svalue, sizeof(svalue), PSTR("RSLT: Receive topic %s, data size %d, data %s"), topicBuf, data_len, dataBuf);
  Serial.println(svalue);

  // Extract command
  memmove(topicBuf, topicBuf+sizeof(MQTT_COMMAND_CHANNEL)-2, sizeof(topicBuf)-sizeof(MQTT_COMMAND_CHANNEL));

  int16_t payload = atoi(dataBuf);     // -32766 - 32767

  if (!strcmp(topicBuf, "power")) {
    Serial.print("power ");
    Serial.println(payload);
    AC_POWER = payload;
  } else if (!strcmp(topicBuf, "mode")) {
    Serial.print("mode ");
    Serial.println(payload);
    AC_MODE = payload;
  } else if (!strcmp(topicBuf, "fan")) {
    Serial.print("fan ");
    Serial.println(payload);
    AC_FAN = payload;
  } else if (!strcmp(topicBuf, "temp")) {
    Serial.print("temp ");
    Serial.println(payload);
    AC_TEMP = payload;
  }

  heatpump->send(irSender, AC_POWER, AC_MODE, AC_FAN, AC_TEMP, AC_VSWING, AC_HSWING);
  publishState();
}

void publishState() {
  char message[MESSZ];
  sprintf(
    message,
    "{\"power\":%d,\"mode\":%d, \"fan\":%d,\"temp\":%d,\"vswing\":%d,\"hswing\":%d}",
    AC_POWER, AC_MODE, AC_FAN, AC_TEMP, AC_VSWING, AC_HSWING
  );
  mqttClient.publish(MQTT_STATUS_CHANNEL, message, true);
}

void reconnectMQTT() {
  // Loop until we're reconnected
  Serial.println("Attempting MQTT connection...");
  // Attempt to connect
  if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS)) {
    Serial.println("connected");
    mqttClient.setCallback(mqttDataCb);
    mqttClient.subscribe(MQTT_COMMAND_CHANNEL);
  } else {
    Serial.print("failed, rc=");
    Serial.println(mqttClient.state());
  }
}



void initOTA() {
  // Port defaults to 8266
   ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
   ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
   ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

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
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}


/*void initWIFI() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
}*/
