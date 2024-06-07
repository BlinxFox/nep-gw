#include <ESPAsyncWebServer.h>
#include "freertos/queue.h"
#include "types.h"

QueueHandle_t nepQueue;

void handleNep(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {

  Serial.printf("%d/%d\n", len, total);
  size_t x;
  for (x = 0; x < len; x++) {
    Serial.printf("%02X ", data[x]);
    if ( x % 16 == 15) {
      Serial.println();
    }
  }
  if ( x % 16 != 15) {
    Serial.println();
  }

  NepItem item;
  getLocalTime(&item.timeinfo);
  memcpy(&item.frame, data, min(len, sizeof(NepFrame)));

  xQueueSend(nepQueue, &item, 0);

  struct tm timeinfo = item.timeinfo;

  char buf[70];
  snprintf(buf, sizeof(buf),
           "%04d%02d%02d%02d%02d%02d",
           timeinfo.tm_year + 1900,
           timeinfo.tm_mon + 1,
           timeinfo.tm_mday,
           timeinfo.tm_hour,
           timeinfo.tm_min,
           timeinfo.tm_sec
          );
  String ret(buf);
  request->send(200, "text/html", ret);

  Serial.println(ret);

}

AsyncWebServer server(80);

void initServer() {
  nepQueue = xQueueCreate(4, sizeof(NepItem));

  server.onNotFound([](AsyncWebServerRequest * request) {
    Serial.printf("Path http://%s%s not found\n", request->host().c_str(), request->url().c_str());
    request->send(404, "text/plain", "404: Not Found");
  });

  server.on("/i.php",     HTTP_POST,
  [](AsyncWebServerRequest * request) {},
  NULL,
  handleNep);
  server.begin();
}
