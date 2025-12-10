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

Controller::Controller(CommandMotor& motor)
    : _motor(motor),
      _mode(ControlMode::MANUAL),
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
        // Future intégration :
        // - StateMachine
        // - Capteurs
    }
}


// ESC [ A  → flèche haut
// ESC [ B  → flèche bas

void Controller::onKey(char key)
{
    static bool esc_detected = false;
    static bool bracket_detected = false;

    // Détection séquence ESC
    if (key == 0x1B) {            // ESC
        esc_detected = true;
        bracket_detected = false;
        return;
    }
    if (esc_detected && key == '[') {
        bracket_detected = true;
        return;
    }
    if (esc_detected && bracket_detected) {
        esc_detected = false;
        bracket_detected = false;

        if (key == 'A') {
            // flèche haut = ASCEND
            onCommand(CommandType::ASCEND);
            return;
        }
        if (key == 'B') {
            // flèche bas = DESCEND
            onCommand(CommandType::DESCEND);
            return;
        }
        return;
    }

    // ------- Touches normales -------
    CommandType cmd = CommandType::NONE;

    switch (key)
    {
        case 'z': case 'Z': cmd = CommandType::FORWARD; break;
        case 'q': case 'Q': cmd = CommandType::TURN_LEFT; break;
        case 'd': case 'D': cmd = CommandType::TURN_RIGHT; break;
        case 's': case 'S': cmd = CommandType::STOP; break;
        case 'a': case 'A': cmd = CommandType::TOGGLE_AUTONOMOUS; break;
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

//
// ---- MODIFICATION : ajout ASCEND / DESCEND ----
//

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

        case CommandType::ASCEND:
            Serial.println("[Controller] MANUAL → ASCEND");
            _motor.ballastVider();
            break;

        case CommandType::DESCEND:
            Serial.println("[Controller] MANUAL → DESCEND");
            _motor.ballastRemplir();
            break;

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
    _motor.setServoAngle(kAngleLeft);
    _motor.setDriverCommand(speed);
}

void Controller::turnRight(float speed)
{
    _motor.setServoAngle(kAngleRight);
    _motor.setDriverCommand(speed);
}

void Controller::stop()
{
    _motor.setDriverCommand(0.0f);
    _motor.setServoAngle(kAngleStraight);
}

// ---- Gestion du mode autonome ----

void Controller::enterAutonomousMode()
{
    _mode = ControlMode::AUTONOMOUS;

    Serial.println("[Controller] Mode AUTONOME ON");

    // comportement simple par défaut
    goStraight(0.5f);
}

void Controller::exitAutonomousMode()
{
    _mode = ControlMode::MANUAL;

    Serial.println("[Controller] Mode MANUEL ON");

    applyManualCommand(_lastManualCmd);
}
