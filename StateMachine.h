#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_

#include <Arduino.h>
#include "CommandMotor.h"
#include "Safety.h"
#include "Capteurs.h"
#include "AsservProfond.h"

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
  // --- CONSTRUCTEUR ---
  // On garde la version complète car tes variables privées (en bas) demandent Capteurs et Safety
  StateMachine(CommandMotor& motor, Capteurs& capteurs, Safety& safety);

  void begin();
  void update();

  // --- GESTION MISSION ---
  void startMission();
  void stopMission();

  bool isRunning() const { return _isRunning; }
  FishState getCurrentState() const { return _currentState; }
  
  // C'est cette fonction qui va permettre au Controller de reprendre la main !
  bool isMissionFinished() const { return _currentState == FishState::COMPLETED; }

  // --- GESTION URGENCE ---
  void setEmergency(EmergencyState e);
  EmergencyState getEmergency() const { return _emergency; }

  // --- SETTERS (Optionnels mais utiles pour les réglages) ---
  void setTargetDepth(float depth) { _targetDepth = depth; }
  void setMoveDuration(unsigned long durationMs) { _moveDuration = durationMs; }
  void setTurnDuration(unsigned long durationMs) { _turnDuration = durationMs; }

private:
  CommandMotor& _motor;
  Capteurs&     _capteurs;
  Safety&       _safety;
  AsservProfond _asserv;

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
  void updateEmergency(); // N'oublie pas d'implémenter celle-ci dans le .cpp si elle manque !

  unsigned long getElapsedTime() const;
  void printStateChange(FishState newState);
};

#endif // STATEMACHINE_H_