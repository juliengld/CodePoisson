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

// Servo 0–180° → 90° = neutre
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
    stop();  // Sécurité
}

void Controller::update()
{
    if (_mode == ControlMode::AUTONOMOUS) {
            // On fait tourner la machine
            _stateMachine.update();
            
            // NOUVEAU : Si la mission est finie, on repasse en manuel
            if (_stateMachine.isMissionFinished()) {
                Serial.println("[Controller] Mission terminée -> Retour en MANUEL");
                exitAutonomousMode();
            }
        }
}

void Controller::onKey(char key)
{
    // ------- Touches normales -------
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
            return; // touche inconnue
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
        // En autonome on ignore les commandes manuelles sauf STOP
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

// ---- Implémentation bas niveau ----

void Controller::goStraight(float speed)
{
    _motor.setServoAngle(kAngleStraight);
    _motor.setDriverCommand(speed);
}


void Controller::turnLeft(float speed)
{
    // Ancienne ligne (à commenter ou supprimer) :
    // _motor.setServoAngle(kAngleLeft);
    
    // NOUVELLE LIGNE : Appelle la fonction du FT90R
    _motor.servoDirectionGauche();
    
    // On active aussi le moteur principal si tu veux avancer en tournant
    _motor.setDriverCommand(speed);
}

void Controller::turnRight(float speed)
{
    // Ancienne ligne (à commenter ou supprimer) :
    // _motor.setServoAngle(kAngleRight);
    
    // NOUVELLE LIGNE : Appelle la fonction du FT90R
    _motor.servoDirectionDroite();
    
    // On active aussi le moteur principal
    _motor.setDriverCommand(speed);
}

void Controller::stop()
{
    _motor.setDriverCommand(0.0f);        // Arrêt propulsion
    _motor.setServoAngle(kAngleStraight); // Arrêt ballast
    
    
    _motor.servoDirectionStop(); 
    //Serial.println("[Controller] MANUAL → STOP (Propulsion + Direction)");
}

// ---- Gestion du mode autonome ----

void Controller::enterAutonomousMode()
{
    _mode = ControlMode::AUTONOMOUS;
        Serial.println("[Controller] Mode AUTONOME ON");

        // Lancement officiel de la mission
        _stateMachine.startMission();
}

void Controller::exitAutonomousMode()
{
    _mode = ControlMode::MANUAL;
        Serial.println("[Controller] Mode MANUEL ON");

        // Arrêt immédiat de la mission autonome
        _stateMachine.stopMission();

        // On réapplique la dernière commande manuelle connue
        applyManualCommand(_lastManualCmd);
}
