#include "StateMachine.h"

// ---- Paramètres par défaut ----
static constexpr float kDefaultTargetDepth = 1.0f;
static constexpr unsigned long kDefaultMoveDuration = 10000;
static constexpr unsigned long kDefaultTurnDuration = 3000;

static constexpr float kMoveSpeed = 0.7f;
static constexpr float kTurnSpeed = 0.6f;

// évite le bug getElapsedTime()==0
static constexpr unsigned long kEntryWindowMs = 50;

StateMachine::StateMachine(CommandMotor& motor, Capteurs& capteurs, Safety& safety)
    : _motor(motor),
      _capteurs(capteurs),
      _safety(safety),
      _currentState(FishState::IDLE),
      _isRunning(false),
      _stateStartTime(0),
      _targetDepth(kDefaultTargetDepth),
      _moveDuration(kDefaultMoveDuration),
      _turnDuration(kDefaultTurnDuration),
      _emergency(EmergencyState::NONE)
{
}

void StateMachine::begin()
{
    Serial.println("[StateMachine] Initialisation");
    _currentState = FishState::IDLE;
    _isRunning = false;
    _stateStartTime = millis();
    _emergency = EmergencyState::NONE;
}

void StateMachine::setEmergency(EmergencyState e)
{
    // latch
    if (_emergency == EmergencyState::NONE) {
        _emergency = e;
    }
}

void StateMachine::update()
{
    // ✅ Safety centralise fuite + batterie + latch + delay
    EmergencyState e = _safety.update(_capteurs);
    if (e != EmergencyState::NONE) {
        setEmergency(e);
    }

    if (_emergency != EmergencyState::NONE && _currentState != FishState::EMERGENCY) {
        changeState(FishState::EMERGENCY);
    }

    // on laisse EMERGENCY tourner même si mission pas lancée
    if (!_isRunning && _currentState != FishState::EMERGENCY) return;

    switch (_currentState)
    {
        case FishState::IDLE:       updateIdle(); break;
        case FishState::DESCENDING: updateDescending(); break;
        case FishState::MOVING:     updateMoving(); break;
        case FishState::TURNING:    updateTurning(); break;
        case FishState::ASCENDING:  updateAscending(); break;
        case FishState::EMERGENCY:  updateEmergency(); break;
        case FishState::COMPLETED:  updateCompleted(); break;
    }
}

void StateMachine::startMission()
{
    Serial.println("[StateMachine] === START MISSION ===");
    _isRunning = true;
    changeState(FishState::DESCENDING);
}

void StateMachine::stopMission()
{
    Serial.println("[StateMachine] === STOP MISSION ===");
    _isRunning = false;
    _motor.setDriverCommand(0.0f);
    changeState(FishState::IDLE);
}

// ==========================================
// États
// ==========================================

void StateMachine::updateIdle()
{
    // rien
}

void StateMachine::updateDescending()
{
    Serial.println("[StateMachine] DESCENTE en cours...");
    _motor.ballastRemplir();

    if (getElapsedTime() > 5000) {
        Serial.println("[StateMachine] Profondeur cible atteinte !");
        changeState(FishState::MOVING);
    }
}

void StateMachine::updateMoving()
{
    if (getElapsedTime() < kEntryWindowMs) {
        Serial.println("[StateMachine] AVANCEMENT démarré");
        _motor.setServoAngle(90.0f);
        _motor.setDriverCommand(kMoveSpeed);
    }

    if (getElapsedTime() >= _moveDuration) {
        Serial.println("[StateMachine] Fin de l'avancement");
        changeState(FishState::TURNING);
    }
}

void StateMachine::updateTurning()
{
    if (getElapsedTime() < kEntryWindowMs) {
        Serial.println("[StateMachine] DEMI-TOUR démarré");
        _motor.setServoAngle(65.0f);
        _motor.setDriverCommand(kTurnSpeed);
    }

    if (getElapsedTime() >= _turnDuration) {
        Serial.println("[StateMachine] Demi-tour terminé");
        changeState(FishState::ASCENDING);
    }
}

void StateMachine::updateAscending()
{
    Serial.println("[StateMachine] REMONTÉE en cours...");
    _motor.ballastVider();

    if (getElapsedTime() > 5000) {
        Serial.println("[StateMachine] Surface atteinte !");
        changeState(FishState::COMPLETED);
    }
}

void StateMachine::updateEmergency()
{
    if (getElapsedTime() < kEntryWindowMs) {
        Serial.println("[StateMachine] === EMERGENCY ===");
        if (_emergency == EmergencyState::LEAK) {
            Serial.println("[StateMachine] Cause: LEAK");
        } else if (_emergency == EmergencyState::BATTERY) {
            Serial.println("[StateMachine] Cause: LOW BATTERY");
        }
    }

    _motor.ballastVider();
    _motor.setServoAngle(90.0f);
    _motor.setDriverCommand(0.0f);
}

void StateMachine::updateCompleted()
{
    if (getElapsedTime() < kEntryWindowMs) {
        Serial.println("[StateMachine] === MISSION TERMINÉE ===");
        _motor.setDriverCommand(0.0f);
        _motor.setServoAngle(90.0f);
    }

    _isRunning = false;
}

// ==========================================
// Helpers
// ==========================================

void StateMachine::changeState(FishState newState)
{
    printStateChange(newState);
    _currentState = newState;
    _stateStartTime = millis();
}

unsigned long StateMachine::getElapsedTime() const
{
    return millis() - _stateStartTime;
}

void StateMachine::printStateChange(FishState newState)
{
    Serial.print("[StateMachine] Transition → ");
    switch (newState)
    {
        case FishState::IDLE:       Serial.println("IDLE"); break;
        case FishState::DESCENDING: Serial.println("DESCENDING"); break;
        case FishState::MOVING:     Serial.println("MOVING"); break;
        case FishState::TURNING:    Serial.println("TURNING"); break;
        case FishState::ASCENDING:  Serial.println("ASCENDING"); break;
        case FishState::COMPLETED:  Serial.println("COMPLETED"); break;
        case FishState::EMERGENCY:  Serial.println("EMERGENCY"); break;
    }
}
