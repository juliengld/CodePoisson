//
//  StateMachine.h
//  Poisson_cpp
//
//  Created by Léa Manu on 10/12/2025.
//

#ifndef StateMachine_h
#define StateMachine_h

#include <Arduino.h>
#include "CommandMotor.h"
#include "Safety.h"

// États possibles du poisson
enum class FishState
{
    IDLE,           // En attente / surface
    DESCENDING,     // Descente avec ballast + PID profondeur
    MOVING,         // Navigation à profondeur stable
    TURNING,        // Demi-tour
    ASCENDING,      // Remontée
    COMPLETED,       // Mission terminée
    EMERGENCY
};

class StateMachine
{
public:
    StateMachine(CommandMotor& motor);
    
    void begin();
    void update();  // À appeler dans la boucle principale
    
    // Démarrage/arrêt de la mission autonome
    void startMission();
    void stopMission();
    
    bool isRunning() const { return _isRunning; }
    FishState getCurrentState() const { return _currentState; }
    
    // Paramètres de mission (à ajuster selon besoins)
    void setTargetDepth(float depth) { _targetDepth = depth; }
    void setMoveDuration(unsigned long durationMs) { _moveDuration = durationMs; }
    void setTurnDuration(unsigned long durationMs) { _turnDuration = durationMs; }

    //Emergency
    void setEmergency(EmergencyState e) { _emergency = e; }
    EmergencyState getEmergency() const { return _emergency; }
    
private:
    CommandMotor& _motor;
    
    FishState _currentState;
    bool _isRunning;
    
    // Timers
    unsigned long _stateStartTime;
    
    // Paramètres de mission
    float _targetDepth;           // Profondeur cible (en mètres ou cm)
    unsigned long _moveDuration;  // Durée d'avancement (ms)
    unsigned long _turnDuration;  // Durée du demi-tour (ms)
    
    // Transitions d'états
    void changeState(FishState newState);
    
    // Méthodes de chaque état
    void updateIdle();
    void updateDescending();
    void updateMoving();
    void updateTurning();
    void updateAscending();
    void updateCompleted();
    
    // Helpers
    unsigned long getElapsedTime() const;
    void printStateChange(FishState newState);

    //Emergency
     EmergencyState _emergency = EmergencyState::NONE;
    void updateEmergency();
};

#endif /* StateMachine_h */
