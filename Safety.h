#pragma once
#include "Capteurs.h"

enum class EmergencyState { NONE, BATTERY, LEAK };

class Safety {
public:
  void begin();
  EmergencyState update(const Capteurs& capteurs);

private:
  EmergencyState _latched = EmergencyState::NONE;
  unsigned long  _lowBatStartMs = 0;
};
