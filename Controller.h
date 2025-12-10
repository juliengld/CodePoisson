//
//  controller.hpp
//  Poisson_cpp
//
//  Created by Léa Manu on 05/12/2025.
//
#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include <Arduino.h>
#include "CommandMotor.h"   // ou CommandMotor.hpp selon fichier réel

// ----- Modes -----
enum class ControlMode {
    MANUAL,
    AUTONOMOUS
};

// ----- Commandes disponibles -----
enum class CommandType {
    NONE,
    FORWARD,
    TURN_LEFT,
    TURN_RIGHT,
    STOP,
    TOGGLE_AUTONOMOUS,
    DESCEND
};

class Controller
{
public:
    // Le controller a besoin d’un accès au moteur :
    Controller(CommandMotor& motor);

    void begin();
    void update();

    // Entrée “clavier” (z, q, d, s, a)
    void onKey(char key);

    // Entrée générique (Wifi ou StateMachine)
    void onCommand(CommandType cmd);

    ControlMode mode() const { return _mode; }

private:
    void applyManualCommand(CommandType cmd);

    // Actions bas niveau :
    void goStraight(float speed);
    void turnLeft(float speed);
    void turnRight(float speed);
    void stop();

    void enterAutonomousMode();
    void exitAutonomousMode();

private:
    CommandMotor& _motor;   // référence vers ton moteur

    ControlMode _mode;
    CommandType _lastManualCmd;
};

#endif


