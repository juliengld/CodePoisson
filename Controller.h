//
//  controller.hpp
//  Poisson_cpp
//
//  Created by Léa Manu on 05/12/2025.
//
#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include <Arduino.h>
#include "CommandMotor.h" 
#include "StateMachine.h"

enum class ControlMode {
    MANUAL,
    AUTONOMOUS
};

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
    Controller(CommandMotor& motor, StateMachine& stateMachine);

    void begin();
    void update();

    void onKey(char key);

    void onCommand(CommandType cmd);

    ControlMode mode() const { return _mode; }

private:
    void applyManualCommand(CommandType cmd);

    void goStraight(float speed);
    void turnLeft(float speed);
    void turnRight(float speed);
    void stop();

    void enterAutonomousMode();
    void exitAutonomousMode();

private:
    CommandMotor& _motor; 

    ControlMode _mode;
    CommandType _lastManualCmd;
    StateMachine& _stateMachine;
};

#endif


