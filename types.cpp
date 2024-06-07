#include "types.h"
#include <Arduino.h>

void decodeNepFrame(NepFrame* frame, NepData* data) {
  auto serial = le32dec(&frame->serial);
  snprintf(data->serial, sizeof(data->serial), "%08X", serial);

  data->currentPower = le16dec(&frame->currentPower1) / 78.6;
  data->voltageDC = le16dec(&frame->voltageDC) / 200.0;
  data->temperature = le16dec(&frame->temperature) / tempA + tempB;
  data->energy = le16dec(&frame->energy) / 4.53;
}
