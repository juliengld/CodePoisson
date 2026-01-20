#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_

#include <Arduino.h>
#include "CommandMotor.h"
#include "Safety.h"
#include "Capteurs.h"

enum class FishState
{
  IDLE,
  DESCENDING,
  MOVING,
  TURNING,
  ASCENDING,
  COMPLETED,
  EMERGENCY
};

class StateMachine
{
public:
  StateMachine(CommandMotor& motor, Capteurs& capteurs, Safety& safety);

  void begin();
  void update();

  void startMission();
  void stopMission();

  bool isRunning() const { return _isRunning; }
  FishState getCurrentState() const { return _currentState; }

  void setEmergency(EmergencyState e);
  EmergencyState getEmergency() const { return _emergency; }

private:
  CommandMotor& _motor;
  Capteurs&     _capteurs;
  Safety&       _safety;

  FishState _currentState = FishState::IDLE;
  bool _isRunning = false;

  unsigned long _stateStartTime = 0;

  float _targetDepth = 1.0f;
  unsigned long _moveDuration = 10000;
  unsigned long _turnDuration = 3000;

  EmergencyState _emergency = EmergencyState::NONE;

  void changeState(FishState newState);

  void updateIdle();
  void updateDescending();
  void updateMoving();
  void updateTurning();
  void updateAscending();
  void updateCompleted();
  void updateEmergency();

  unsigned long getElapsedTime() const;
  void printStateChange(FishState newState);
};

#endif // STATEMACHINE_H_
