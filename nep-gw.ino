
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <ETH.h>
#include "ESP32MQTTClient.h"

#include "freertos/queue.h"

#include "types.h"

// const char *ssid = "NEP-GW";
// const char *password = "11111111";

const char* nepServer = "www.nepviewer.net";
static bool eth_connected = false;

const byte DNS_PORT = 53;
IPAddress apIP(172, 0, 0, 1);
DNSServer dnsServer;

extern QueueHandle_t nepQueue;

constexpr auto numInverters = 4;
uint32_t inverters[numInverters] = {0};

void setup() {
  Serial.begin(115200);

  WiFi.onEvent(WiFiEvent);
  ETH.begin();

  Serial.println("Starting AP");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  dnsServer.start(DNS_PORT, "*", apIP);

  Serial.println("Starting server");
  initServer();
  Serial.println("Starting MQTT");
  initMqtt();

  Serial.println("Startup done!");

  Serial.printf("TempA: %f\n", tempA);
  Serial.printf("TempB: %f\n", tempB);
}

#include "time.h"

void loop() {
  if (eth_connected) {
    if (ntp_needTime()) {
      Serial.println("Update Time");
      ntp_update();
    }
  }
  dnsServer.processNextRequest();

  while (uxQueueMessagesWaiting(nepQueue) > 0) {
    NepItem item;
    xQueueReceive(nepQueue, &item, 0);

    forwardData(item);

  }
  delay(20);
}


void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("ETH Started");
      //set eth hostname here
      ETH.setHostname("esp32-ethernet");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;

    case ARDUINO_EVENT_WIFI_READY:
      Serial.println("ARDUINO_EVENT_WIFI_READY");
      break;
    case ARDUINO_EVENT_WIFI_AP_START:
      Serial.println("ARDUINO_EVENT_WIFI_AP_START");
      break;
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
      Serial.println("ARDUINO_EVENT_WIFI_AP_STACONNECTED");
      break;
    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
      Serial.println("ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED");
      break;

    default:
      Serial.printf("unknown Event: %d\n", event);
      break;
  }
}

#include <endian.h>

void printNep(NepData *data) {
  Serial.print("serial ");
  Serial.println(data->serial);

  Serial.print("voltageDC ");
  Serial.println(data->voltageDC);

  Serial.print("Wh ");
  Serial.println(data->energy);

  Serial.print("currentPower ");
  Serial.println(data->currentPower);

  Serial.print("temperature ");
  Serial.println(data->temperature);
}

void forwardData(NepItem &item) {
  WiFiClient client;
  char buf[128];
  snprintf(buf, sizeof(buf), "POST /i.php HTTP/1.1\r\n"
           "Host: %s\r\n"
           "Connection: close\r\n"
           "Content-Length: %d\r\n"
           "\r\n", nepServer, sizeof(NepFrame));
  Serial.println(buf);

  client.connect(nepServer, 80);
  client.print(buf);
  client.write((uint8_t*) &item.frame, sizeof(NepFrame));

  while (client.connected()) {
    while (client.available()) {
      char d = client.read();
      Serial.print(d);
    }
  }
  Serial.println();

  NepData data;
  decodeNepFrame(&item.frame, &data);

  printNep(&data);

  auto serial = le32dec(&item.frame.serial);
  bool found = false;
  for (int x = 0; x < numInverters; x++) {
    if ( serial == inverters[x]) {
      found = true;
    }
  }

  if (!found) {
    Serial.printf("New inverter found: %08X\n", serial);
    for (int x = 0; x < numInverters; x++) {
      if (inverters[x] == 0) {
        inverters[x] = serial;
        registerInverter(serial);
        break;
      }
    }
  }
  publishData(serial, &data);
}
