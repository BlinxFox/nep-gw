#include "time.h"

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 0;//3600;

unsigned long ntp_lastUpdate = 0;
bool ntp_needTime() {
  auto dx = millis() - ntp_lastUpdate;

  return ntp_lastUpdate == 0 || dx > (24 * 60 * 60 * 1000);
}

void ntp_update() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  ntp_lastUpdate = millis();
}
