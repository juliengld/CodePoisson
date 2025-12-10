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

void CommandMotor::ballastSuivreProfondeur(float targetDepth_m, float currentDepth_m)
{
    if (!servo_ok) {
        return;
    }

    float err = targetDepth_m - currentDepth_m;

    float Kp_simple = 5.0f; // à ajuster / remplacer par ton calcul géométrique
    float angleCmdDeg = 90.0f + Kp_simple * err;  // 90° = position neutre fictive

    if (angleCmdDeg < 0.0f)   angleCmdDeg = 0.0f;
    if (angleCmdDeg > 180.0f) angleCmdDeg = 180.0f;

    setServoAngle(angleCmdDeg);
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
    // Saturation [-1 ; 1]
    if (command >  1.0f) command = 1.0f;
    if (command < -1.0f) command = -1.0f;

    uint8_t pwm = (uint8_t)(fabs(command) * 255.0f);

    if (command > 0.0f) {
        analogWrite(DRIVER_PWM_A, pwm);
        analogWrite(DRIVER_PWM_B, 0);
    }
    else if (command < 0.0f) {
        analogWrite(DRIVER_PWM_A, 0);
        analogWrite(DRIVER_PWM_B, pwm);
    }
    else {
        analogWrite(DRIVER_PWM_A, 0);
        analogWrite(DRIVER_PWM_B, 0);
    }
}
