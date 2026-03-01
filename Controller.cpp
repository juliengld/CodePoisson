//
//  controller.cpp
//  Poisson_cpp
//
//  Created by Léa Manu on 05/12/2025.
//

#include "Controller.h"

// ---- Paramètres généraux ----
static constexpr float kForwardSpeed = 0.8f;     // 80%
static constexpr float kTurnSpeed    = 0.6f;     // 60%

//  90° = neutre
static constexpr float kAngleStraight = 90.0f;
static constexpr float kAngleLeft     = 90.0f - 25.0f;
static constexpr float kAngleRight    = 90.0f + 25.0f;

Controller::Controller(CommandMotor& motor, StateMachine& stateMachine)
    : _motor(motor),
      _mode(ControlMode::MANUAL),
      _stateMachine(stateMachine),
      _lastManualCmd(CommandType::STOP)
{
}

void Controller::begin()
{
    Serial.println("[Controller] Initialisation");
    stop(); 
}

void Controller::update()
{
    if (_mode == ControlMode::AUTONOMOUS) {
            _stateMachine.update();
            if (_stateMachine.isMissionFinished()) {
                Serial.println("[Controller] Mission terminée -> Retour en MANUEL");
                exitAutonomousMode();
            }
        }
}

void Controller::onKey(char key)
{
    CommandType cmd = CommandType::NONE;

    switch (key)
    {
        case 'z': case 'Z': cmd = CommandType::FORWARD; break;
        case 'q': case 'Q': cmd = CommandType::TURN_LEFT; break;
        case 'd': case 'D': cmd = CommandType::TURN_RIGHT; break;
        case 's': case 'S': cmd = CommandType::STOP; break;
        case 'a': case 'A': cmd = CommandType::TOGGLE_AUTONOMOUS; break;
        //case 'f': case 'F': cmd = CommandType::DESCEND; break;
        default:
            return; 
    }

    onCommand(cmd);
}

void Controller::onCommand(CommandType cmd)
{
    if (cmd == CommandType::TOGGLE_AUTONOMOUS) {

        if (_mode == ControlMode::MANUAL) enterAutonomousMode();
        else                               exitAutonomousMode();

        return;
    }

    if (_mode == ControlMode::MANUAL) {
        applyManualCommand(cmd);
    }
    else {
        if (cmd == CommandType::STOP) stop();
    }
}

void Controller::applyManualCommand(CommandType cmd)
{
    _lastManualCmd = cmd;

    switch (cmd)
    {
        case CommandType::FORWARD:
            goStraight(kForwardSpeed);
            Serial.println("[Controller] MANUAL → FORWARD");
            break;

        case CommandType::TURN_LEFT:
            turnLeft(kTurnSpeed);
            Serial.println("[Controller] MANUAL → LEFT");
            break;

        case CommandType::TURN_RIGHT:
            turnRight(kTurnSpeed);
            Serial.println("[Controller] MANUAL → RIGHT");
            break;

        case CommandType::STOP:
            stop();
            Serial.println("[Controller] MANUAL → STOP");
            break;
// A voir mais pour le moment c'est commenté 
        //case CommandType::DESCEND:
        //    Serial.println("[Controller] MANUAL → DESCEND (ballastRemplir)");
        //    _motor.ballastRemplir();
        //    break;

        default:
            break;
    }
}


void Controller::goStraight(float speed)
{
    _motor.setServoAngle(kAngleStraight);
    _motor.setDriverCommand(speed);
}


void Controller::turnLeft(float speed)
{
    // Ancienne ligne:
    // _motor.setServoAngle(kAngleLeft);

    _motor.servoDirectionGauche();
    _motor.setDriverCommand(speed);
}

void Controller::turnRight(float speed)
{
    // Ancienne ligne :
    // _motor.setServoAngle(kAngleRight);
    _motor.servoDirectionDroite();
    _motor.setDriverCommand(speed);
}

void Controller::stop()
{
    _motor.setDriverCommand(0.0f);        // Arrêt propulsion
    _motor.setServoAngle(kAngleStraight); // Arrêt ballast
    
    
    _motor.servoDirectionStop(); 
    //Serial.println("[Controller] MANUAL → STOP (Propulsion + Direction)");
}

void Controller::enterAutonomousMode()
{
    _mode = ControlMode::AUTONOMOUS;
        Serial.println("[Controller] Mode AUTONOME ON");

        _stateMachine.startMission();
}

void Controller::exitAutonomousMode()
{
    _mode = ControlMode::MANUAL;
        Serial.println("[Controller] Mode MANUEL ON");

        _stateMachine.stopMission();

        applyManualCommand(_lastManualCmd);
}
