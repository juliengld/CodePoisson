//
//  StateMachine.cpp
//  Poisson_cpp
//
//  Created by Léa Manu on 10/12/2025.
//

#include "StateMachine.h"

// ---- Paramètres par défaut ----
static constexpr float kDefaultTargetDepth = 1.0f;      // 1 mètre de profondeur
static constexpr unsigned long kDefaultMoveDuration = 10000;  // 10 secondes d'avancement
static constexpr unsigned long kDefaultTurnDuration = 3000;   // 3 secondes de demi-tour

static constexpr float kMoveSpeed = 0.7f;      // Vitesse d'avancement
static constexpr float kTurnSpeed = 0.6f;      // Vitesse de rotation

StateMachine::StateMachine(CommandMotor& motor)
    : _motor(motor),
      _currentState(FishState::IDLE),
      _isRunning(false),
      _stateStartTime(0),
      _targetDepth(kDefaultTargetDepth),
      _moveDuration(kDefaultMoveDuration),
      _turnDuration(kDefaultTurnDuration)
{
}

void StateMachine::begin()
{
    Serial.println("[StateMachine] Initialisation");
    _currentState = FishState::IDLE;
    _isRunning = false;
}

void StateMachine::update()
{

    if (_emergency != EmergencyState::NONE && _currentState != FishState::EMERGENCY) {
        changeState(FishState::EMERGENCY);
    }

    if (!_isRunning) return;
    
    // Machine à états principale
    switch (_currentState)
    {
        case FishState::IDLE:
            updateIdle();
            break;
            
        case FishState::DESCENDING:
            updateDescending();
            break;
            
        case FishState::MOVING:
            updateMoving();
            break;
            
        case FishState::TURNING:
            updateTurning();
            break;
            
        case FishState::ASCENDING:
            updateAscending();
            break;

        case FishState::EMERGENCY:
            updateEmergency();
            break;
            
        case FishState::COMPLETED:
            updateCompleted();
            break;
    }
}

void StateMachine::startMission()
{
    Serial.println("[StateMachine] === DÉMARRAGE MISSION AUTONOME ===");
    _isRunning = true;
    changeState(FishState::DESCENDING);
}

void StateMachine::stopMission()
{
    Serial.println("[StateMachine] === ARRÊT MISSION ===");
    _isRunning = false;
    _motor.setDriverCommand(0.0f);
    changeState(FishState::IDLE);
}

// ==========================================
// GESTION DES ÉTATS
// ==========================================

void StateMachine::updateIdle()
{
    // En attente, rien à faire
    // La mission démarre via startMission()
}

void StateMachine::updateDescending()
{
    // TODO: Intégration avec le fichier d'asservissement de plongée
    // Pour l'instant : simulation simple
    
    Serial.println("[StateMachine] DESCENTE en cours...");
    
    // Appel de la fonction de descente du ballast
    _motor.ballastRemplir();
    
    // TODO: Ton collègue devra implémenter quelque chose comme :
    // if (AsservissementPlongeon::isAtDepth(_targetDepth)) {
    //     changeState(FishState::MOVING);
    // }
    
    // SIMULATION : après 5 secondes, on considère qu'on est à la bonne profondeur
    if (getElapsedTime() > 5000) {
        Serial.println("[StateMachine] Profondeur cible atteinte !");
        changeState(FishState::MOVING);
    }
}

void StateMachine::updateMoving()
{
    // Avancer tout droit pendant _moveDuration
    
    if (getElapsedTime() == 0) {
        Serial.println("[StateMachine] AVANCEMENT démarré");
        _motor.setServoAngle(90.0f);  // Tout droit
        _motor.setDriverCommand(kMoveSpeed);
    }
    
    // TODO: Le PID de profondeur devrait tourner en continu ici
    // AsservissementPlongeon::maintainDepth(_targetDepth);
    
    if (getElapsedTime() >= _moveDuration) {
        Serial.println("[StateMachine] Fin de l'avancement");
        changeState(FishState::TURNING);
    }
}

void StateMachine::updateTurning()
{
    // Faire un demi-tour pendant _turnDuration
    
    if (getElapsedTime() == 0) {
        Serial.println("[StateMachine] DEMI-TOUR démarré");
        // Tourner à gauche (tu peux changer pour droite)
        _motor.setServoAngle(65.0f);  // Angle de braquage
        _motor.setDriverCommand(kTurnSpeed);
    }
    
    // TODO: Maintenir la profondeur pendant le virage
    // AsservissementPlongeon::maintainDepth(_targetDepth);
    
    if (getElapsedTime() >= _turnDuration) {
        Serial.println("[StateMachine] Demi-tour terminé");
        changeState(FishState::ASCENDING);
    }
}

void StateMachine::updateAscending()
{
    // TODO: Intégration avec le fichier d'asservissement de remontée
    
    Serial.println("[StateMachine] REMONTÉE en cours...");
    
    // Vider le ballast pour remonter
    _motor.ballastVider();
    
    // TODO: Ton collègue devra implémenter :
    // if (AsservissementPlongeon::isAtSurface()) {
    //     changeState(FishState::COMPLETED);
    // }
    
    // SIMULATION : après 5 secondes, on considère qu'on est remonté
    if (getElapsedTime() > 5000) {
        Serial.println("[StateMachine] Surface atteinte !");
        changeState(FishState::COMPLETED);
    }
}

void StateMachine::updateEmergency()
{
    // On remonte coûte que coûte
    if (getElapsedTime() == 0) {
        Serial.println("[StateMachine] === EMERGENCY ===");
        if (_emergency == EmergencyState::LEAK) {
            Serial.println("[StateMachine] Cause: LEAK");
        } else if (_emergency == EmergencyState::BATTERY) {
            Serial.println("[StateMachine] Cause: LOW BATTERY");
        }
    }

    // Remontée : ballast en vidange
    _motor.ballastVider();

    // Sécurité propulsion / direction (à ajuster selon ton choix)
    _motor.setServoAngle(90.0f);     // neutre
    _motor.setDriverCommand(0.0f);   // stop moteur principal

    // Option : tu peux décider de passer en COMPLETED après X secondes,
    // ou rester en EMERGENCY jusqu'à reset manuel.
    // Ici : on reste en EMERGENCY (le plus safe).
}


void StateMachine::updateCompleted()
{
    if (getElapsedTime() == 0) {
        Serial.println("[StateMachine] === MISSION TERMINÉE ===");
        _motor.setDriverCommand(0.0f);
        _motor.setServoAngle(90.0f);
    }
    
    // Mission terminée, on reste en surface
    _isRunning = false;
}

// ==========================================
// HELPERS
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
