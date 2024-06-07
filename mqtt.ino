#include "ESP32MQTTClient.h"

#include<WiFi.h>
#include<ArduinoJson.h>

// const char *mqttServer = "mqtt://192.168.0.100:1883";
const char *topicDiscover = "homeassistant/sensor/nep-gw_%s/inverter_%08X_%s/config";
const char *topicPublish = "nep-gw_%s/inverter_%08X/state";

ESP32MQTTClient mqttClient;

void initMqtt() {
  mqttClient.enableDebuggingMessages();
  mqttClient.setURI(mqttServer);
  mqttClient.enableLastWillMessage("lwt", "I am going offline");
  mqttClient.setKeepAlive(30);
  mqttClient.loopStart();
}

esp_err_t handleMQTT(esp_mqtt_event_handle_t event)
{
  Serial.println("MQTT event");
  mqttClient.onEventCallback(event);
  return ESP_OK;
}

void onConnectionEstablishedCallback(esp_mqtt_client_handle_t client)
{
  Serial.println("MQTT connected");
}

void registerInverter(uint32_t serial) {
  char topic[128];
  char buf[256];

  uint8_t mac[6] = {0, 0, 0, 0, 0, 0};
  char macStr[13] = {0};
  WiFi.macAddress(mac);
  snprintf(macStr, sizeof(macStr), "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  snprintf(topic, sizeof(topic), topicDiscover, macStr, serial, "energy");

  snprintf(buf, sizeof(buf), "NEP-%08X", serial);
  String deviceName = String(buf);

  snprintf(buf, sizeof(buf), topicPublish, macStr, serial);
  String topicState = String(buf);

  JsonDocument payload;
  JsonObject device;
  JsonArray identifiers;
  String strPayload;

  snprintf(topic, sizeof(topic), topicDiscover, macStr, serial, "energy");

  payload["name"] = deviceName + ".energy";
  payload["state_topic"] = topicState;
  payload["state_class"] = "total_increasing";
  payload["device_class"] = "energy";
  payload["unit_of_measurement"] = "Wh";
  payload["value_template"] = "{{ value_json.energy }}";
  payload["unique_id"] = deviceName + "_energy";

  device = payload["device"].to<JsonObject>();
  device["name"] = deviceName;

  identifiers = device["identifiers"].to<JsonArray>();
  identifiers.add(deviceName);

  serializeJson(payload, strPayload);

  Serial.println(topic);
  Serial.println(strPayload);

  mqttClient.publish(topic, strPayload, 0, false);



  snprintf(topic, sizeof(topic), topicDiscover, macStr, serial, "temp");

  payload["name"] = deviceName + ".temp";
  payload["state_topic"] = topicState;
  payload["state_class"] = "measurement";
  payload["device_class"] = "temperature";
  payload["unit_of_measurement"] = "°C";
  payload["value_template"] = "{{ value_json.temperature }}";
  payload["unique_id"] = deviceName + "_temp";

  serializeJson(payload, strPayload);

  Serial.println(topic);
  Serial.println(strPayload);

  mqttClient.publish(topic, strPayload, 0, false);



  snprintf(topic, sizeof(topic), topicDiscover, macStr, serial, "voltage");

  payload["name"] = deviceName + ".voltage";
  payload["state_topic"] = topicState;
  payload["state_class"] = "measurement";
  payload["device_class"] = "voltage";
  payload["unit_of_measurement"] = "V";
  payload["value_template"] = "{{ value_json.voltageDC }}";
  payload["unique_id"] = deviceName + "_voltage";

  serializeJson(payload, strPayload);

  Serial.println(topic);
  Serial.println(strPayload);

  mqttClient.publish(topic, strPayload, 0, false);



  snprintf(topic, sizeof(topic), topicDiscover, macStr, serial, "power");

  payload["name"] = deviceName + ".power";
  payload["state_topic"] = topicState;
  payload["state_class"] = "measurement";
  payload["device_class"] = "power";
  payload["unit_of_measurement"] = "W";
  payload["value_template"] = "{{ value_json.currentPower }}";
  payload["unique_id"] = deviceName + "_power";

  serializeJson(payload, strPayload);

  Serial.println(topic);
  Serial.println(strPayload);

  mqttClient.publish(topic, strPayload, 0, false);

}

void publishData(uint32_t serial, NepData *data) {
  char buf[256];
  uint8_t mac[6] = {0, 0, 0, 0, 0, 0};
  char macStr[13] = {0};
  WiFi.macAddress(mac);
  snprintf(macStr, sizeof(macStr), "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  snprintf(buf, sizeof(buf), topicPublish, macStr, serial);
  String topicState = String(buf);

  JsonDocument payload;
  payload["energy"] = data->energy;
  payload["temperature"] = data->temperature;
  payload["voltageDC"] = data->voltageDC;
  payload["currentPower"] = data->currentPower;

  String strPayload;
  serializeJson(payload, strPayload);

  Serial.println(topicState);
  Serial.println(strPayload);

  mqttClient.publish(topicState, strPayload, 0, false);
}
