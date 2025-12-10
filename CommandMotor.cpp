#include "CommandMotor.h"
#include <Servo.h>

CommandMotor::CommandMotor()
{
    servo_ok = false;
    servoDirection_ok = false; // 2e servo non initialisé par défaut
}

bool CommandMotor::begin()
{
    // ----- Servo ballast sur D0 -----
    if (servo.attach(SERVO_PIN, pulseMin_us, pulseMax_us)) {
        servo_ok = true;
        Serial.println("[OK] Servo SER0067 attaché sur D0");
    } else {
        servo_ok = false;
        Serial.println("[ERREUR] Impossible d’attacher le SER0067 sur D0");
    }

    // ----- Servo direction sur SERVO_DIRECTION_PIN -----
    // NOTE : seulement la structure ici, tu pourras compléter la logique plus tard
    if (servoDirection.attach(SERVO_DIRECTION_PIN, pulseMin_us, pulseMax_us)) {
        servoDirection_ok = true;
        Serial.println("[OK] Servo direction attaché");
    } else {
        servoDirection_ok = false;
        Serial.println("[ERREUR] Impossible d’attacher le servo direction");
    }

    // ----- Driver 2x PWM sur D4 / D5 -----
    pinMode(DRIVER_PWM_A, OUTPUT);
    pinMode(DRIVER_PWM_B, OUTPUT);

    analogWrite(DRIVER_PWM_A, 0);
    analogWrite(DRIVER_PWM_B, 0);

    Serial.println("[OK] Driver PWM initialisé sur D4/D5");

    // même si les servos échouent, on ne bloque pas
    return true;
}

void CommandMotor::setServoAngle(float angleDeg)
{
    if (!servo_ok) return;

    if (angleDeg < 0.0f)   angleDeg = 0.0f;
    if (angleDeg > 180.0f) angleDeg = 180.0f;

    servo.write(angleDeg);
}

// ============================================================
//   GESTION BALLAST PAR SERVO + CREMAILLERE
// ============================================================

void CommandMotor::ballastVider()
{
    if (!servo_ok) {
        return;
    }

    float angleEmptyDeg = 0.0f; // à ajuster avec ta géométrie
    setServoAngle(angleEmptyDeg);
}

void CommandMotor::ballastRemplir()
{
    if (!servo_ok) {
        return;
    }

    float angleFullDeg = 180.0f; // valeur fictive, à ajuster
    setServoAngle(angleFullDeg);
}
void CommandMotor::ballastEquilibre()
{
    if (!servo_ok) {
        return;
    }

    float angleFullDeg = 30.0f; // valeur fictive, à ajuster
    setServoAngle(angleFullDeg);
}

// ============================================================
//   GESTION SERVO DE DIRECTION (2e servo) – SQUELETTE SEULEMENT
// ============================================================

void CommandMotor::servoDirectionDroite()
{
    // TODO: implémenter l’angle / la commande pour tourner à droite
    // Exemple futur :
    //   if (!servoDirection_ok) return;
    //   servoDirection.write(angleDroiteDeg);
}

void CommandMotor::servoDirectionGauche()
{
    // TODO: implémenter l’angle / la commande pour tourner à gauche
    // Exemple futur :
    //   if (!servoDirection_ok) return;
    //   servoDirection.write(angleGaucheDeg);
}

// ============================================================
//   DRIVER 2x PWM
// ============================================================

void CommandMotor::setDriverRaw(uint8_t pwm4, uint8_t pwm5)
{
    analogWrite(DRIVER_PWM_A, pwm4);
    analogWrite(DRIVER_PWM_B, pwm5);
}

void CommandMotor::setDriverCommand(float command)
{
    // Commande normalisée [0 ; 1]
    if (command < 0.0f) command = 0.0f;
    if (command > 1.0f) command = 1.0f;

    // Conversion en PWM 0–255
    uint8_t pwm = (uint8_t)(command * 255.0f + 0.5f);

    // Moteur UNIQUEMENT en marche avant : D4 = PWM, D5 = 0
    setDriverRaw(pwm, 0);
}
