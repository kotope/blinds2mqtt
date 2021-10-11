// DIY Smart Blinds Controller for ESP8266 (Wemos D1 Mini)
// Supports Home Assistant MQTT auto discovery straight out of the box
// (c) Toni Korhonen 2021
// https://www.creatingsmarthome.com/?p=629

#include <ESP8266WiFi.h>
#include <Servo.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>

#include "blinds2mqtt.h"
#include "BlindsServo.h"

#define SW_VERSION "0.2.1"

#define JSON_BUFFER_LENGTH 2048
#define MQTT_TOPIC_MAX_LENGTH 256

#define DEBUG true // default value for debug

// Callbacks
void mqttCallback(char* topic, byte* payload, unsigned int payloadLength);

// Wifi
WiFiClient wifiClient;

// MQTT
PubSubClient client(mqtt_server, mqtt_port, mqttCallback, wifiClient);

BlindsServo servos[sizeof(servoPins)];

// debug
bool debug = DEBUG;
int numberOfServos = 0;
int pos = 0;

String uniqueId;

void setup() {
  // Setup serial port
  Serial.begin(115200);

  uniqueId = WiFi.macAddress();
  uniqueId.replace(":", "-");

  wifiConnect();

  // Setup OTA
  initOTA();

  numberOfServos = sizeof(servoPins) / sizeof(servoPins[0]);
  int numberOfReversedServos = sizeof(reversedPins) / sizeof(servoPins[0]); // We might not have any servos as reversed, thus using servoPin size

  Serial.print("numberOfServos = ");
  Serial.println(numberOfServos);

  for (int i = 0; i < numberOfServos; i++) {
    unsigned int servoPin = servoPins[i];
    boolean reversed = false;

    // Check if servo is in reversed array
    for (int r = 0; r < numberOfReversedServos; r++) {
      unsigned int reversedPin = reversedPins[r];
      if(servoPin == reversedPin) {
        reversed = true;
        break;
      }
    }

    servos[i] = BlindsServo(i+1, servoPin, servo_min_pulse, servo_max_pulse, servo_max_angle, reversed, debug);
    servos[i].setDebugPrintCallback(debugPrint);
    servos[i].setStatusChangedCallback(statusChanged);
  }

  mqttConnect();
  client.setCallback(mqttCallback);
}

void wifiConnect() {
  Serial.println("Connecting wifi..");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    // wait 500ms
    Serial.print(".");
    delay(500);
  }

  WiFi.mode(WIFI_STA);

  // Connected to WiFi
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
}

void mqttConnect() {
  if (!!!client.connected()) {
    Serial.println("Try to connect mqtt..");
    int count = 20;
    while (count-- > 0 && !!!client.connect(client_id, mqtt_username, mqtt_password)) {
      delay(500);
    }

    for (int i = 0; i < numberOfServos; i++) {
      BlindsServo s = servos[i];
      Serial.print("Publishing ha config for servo ");
      Serial.println(String(s.getId()));

      subscribeAndPublishConfig(s.getId());
    }
  }
}


void initOTA() {
  ArduinoOTA.setHostname(client_id);
  ArduinoOTA.setPassword(ota_password);
  ArduinoOTA.begin();
}

void loop()
{
  // check that we are connected
  if (!client.loop()) {
    mqttConnect();
  }

  for (int i = 0; i < numberOfServos; i++) {
    servos[i].loop();
  }

  // Wait for servos to turn
  delay(15);

  // Rest of the loop
  MDNS.update();
  ArduinoOTA.handle();
}

BlindsServo& servoById(int id) {
  for (int i = 0; i < numberOfServos; i++) {
    BlindsServo& s = servos[i];
    if (s.getId() == id) {
      return s;
    }
  }
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

// MQTT
void subscribeAndPublishConfig(int servoId) {
  // Subscribe to servo topic
  char topic[MQTT_TOPIC_MAX_LENGTH];
  sprintf(topic, blinds_command_topic, uniqueId.c_str(), servoId);

  if (client.subscribe(topic)) {
    // OK
    debugPrint("Subscribed to blinds set topic (" + String(servoId) + "), uniqueId = " + uniqueId);
    publishConfig(servoId); // Publish config right after subscribed to command topic
  } else {
    // FAIL
    debugPrint("Failed to subscribe to blinds set topic (" + String(servoId) + ")");
  }
}

void mqttCallback(char* topic, byte * payload, unsigned int length) {
  Serial.print("Got MQTT callback! ");
  Serial.print("topic = ");
  Serial.println(topic);
  
  String myString = String(topic);

  String v = getValue(topic, '/', 2); // Get the second value
  if (!v.length()) {
    return;
  }
  
  int servoId = v.toInt(); // Get servo id

  // Check
  if (getValue(topic, '/', 0) == "blinds") { //Ensure blinds topic
     if(getValue(topic, '/', 1) == uniqueId) { // Ensure correct device
       if(getValue(topic, '/', 3) == "set") { // Ensure set topic
           handleSet(payload, length, servoId);
       }
     }
  }
}

void handleSet(byte * payload, unsigned int length, int servoId) {
  BlindsServo& s = servoById(servoId);    
  if (!strncmp((char *)payload, "OPEN", length)) {
    s.setOpen();
  } else if (!strncmp((char *)payload, "CLOSE", length)) {
    s.setClose();
  } else if (!strncmp((char *)payload, "STOP", length)) { 
    s.setStop();
  }
}

void publishConfig(int servoId) {
  Serial.println("Publishing ha config.");

  DynamicJsonDocument root(JSON_BUFFER_LENGTH);
  
  // State topic
  char state_t[MQTT_TOPIC_MAX_LENGTH];
  sprintf(state_t, blinds_state_topic, uniqueId.c_str(), servoId);
  root["state_topic"] = state_t;

  // Command topic
  char command_t[MQTT_TOPIC_MAX_LENGTH];
  sprintf(command_t, blinds_command_topic, uniqueId.c_str(), servoId);
  root["command_topic"] = command_t;
  
  // Others
  root["name"] = numberOfServos > 1 ? friendly_name + " " + String(servoId) : friendly_name;
  root["device_class"] = "blind";
  root["unique_id"] = "blinds/" + uniqueId + "/servo" + String(servoId);

  // Device
  addDevice(root);

  // Publish
  String mqttOutput;
  serializeJson(root, mqttOutput);
  
  char t[MQTT_TOPIC_MAX_LENGTH];
  sprintf(t, ha_config_topic, uniqueId.c_str(), servoId);
  client.beginPublish(t, mqttOutput.length(), true); 
  client.print(mqttOutput);
  client.endPublish();
}

void addDevice(DynamicJsonDocument& root) {
  JsonObject device = root.createNestedObject("device");
  
  JsonArray identifiers = device.createNestedArray("identifiers");
  identifiers.add(uniqueId.c_str());
  
  device["name"] = "blinds2mqtt";
  device["model"] = "esp8266";
  device["sw_version"] = SW_VERSION;
}

//Callbacks
void debugPrint(String message) {
  if (debug) {
    Serial.println(message);
    // publish to debug topic
    char t[MQTT_TOPIC_MAX_LENGTH];
    sprintf(t, blinds_debug_topic, uniqueId.c_str());
    client.publish(t, message.c_str());
  }
}


void statusChanged(int servoId) {
  BlindsServo& s = servoById(servoId);

  String statusMsg = "OPEN";
  
  switch(s.getStatus()) {
    case BlindsServo::OPEN:
      statusMsg = "open";
      break;
    case BlindsServo::CLOSED:
      statusMsg = "closed";
      break;
    case BlindsServo::CLOSING:
      statusMsg = "closing";
      break;
    case BlindsServo::OPENING:
      statusMsg = "opening";
      break;
    default:
      break;
  }
  
  char t[MQTT_TOPIC_MAX_LENGTH];
  sprintf(t, blinds_state_topic, uniqueId.c_str(), servoId);
  client.publish(t, statusMsg.c_str());
}
