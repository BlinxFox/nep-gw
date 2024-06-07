#pragma once

#include <stdint.h>
#include <time.h>
#include <endian.h>

struct __attribute__((__packed__)) NepFrame {
  char unknown1[19];
  uint32_t serial;
  char unknown2[2];
  uint16_t currentPower1; // div 78.6
  uint16_t voltageDC; // div 200
  char unknown3[2];
  uint16_t currentPower2; // div 40
  char unknown4[2];
  uint16_t temperature; // div 158 // y = A * x + b  --- A = 1/72, B = -32
  uint16_t energy; // div 4.53  since start
  char unknown5[6];
};

struct NepItem {
  struct tm timeinfo;
  struct NepFrame frame;
};

struct NepData {
  char serial[9];
  float currentPower;
  float voltageDC;
  float temperature;
  float energy;
};

constexpr auto tempY1 = 15.0;
constexpr auto tempX1 = 3359;

constexpr auto tempY2 = 26.4;
constexpr auto tempX2 = 0x0edf;

constexpr auto tempA = (tempX1 - tempX2) / (tempY1 - tempY2);
constexpr auto tempB = tempY2 - tempX2 / tempA;

void decodeNepFrame(NepFrame* frame, NepData* data);
