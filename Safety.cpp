#include "Safety.h"
#include <Arduino.h>

static constexpr float kBatTripPercent = 15.0f;
static constexpr unsigned long kBatDelayMs = 4000;

void Safety::begin() {
  _latched = EmergencyState::NONE;
  _lowBatStartMs = 0;
}

EmergencyState Safety::update(const Capteurs& capteurs)
{
  if (_latched != EmergencyState::NONE)
    return _latched;

  // 1) FUITE (API chez toi = getLeakData)
  auto leak = capteurs.getLeakData();
  if (leak.leakLatched) {
    _latched = EmergencyState::LEAK;
    return _latched;
  }

  // 2) BATTERIE (on lit juste le % via une méthode)
  float batPercent = capteurs.getBatteryPercent();

// ✅ Si le capteur batterie n'est pas dispo / pas initialisé, on ignore la condition batterie.
// On considère "invalide" : <= 0, > 100, ou NaN.
if (!(batPercent > 0.0f && batPercent <= 100.0f)) {
  _lowBatStartMs = 0;
  return EmergencyState::NONE;
}

if (batPercent < kBatTripPercent) {
  if (_lowBatStartMs == 0) _lowBatStartMs = millis();
  if (millis() - _lowBatStartMs >= kBatDelayMs) {
    _latched = EmergencyState::BATTERY;
    return _latched;
  }
} else {
  _lowBatStartMs = 0;
}


  return EmergencyState::NONE;
}
