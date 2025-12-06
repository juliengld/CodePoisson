#include "CommandMotor.h"
#include <Servo.h>

CommandMotor::CommandMotor()
{
    servo_ok = false;
}

bool CommandMotor::begin()
{
    // ----- Servo sur D0 -----
    if (servo.attach(SERVO_PIN, pulseMin_us, pulseMax_us)) {
        servo_ok = true;
        Serial.println("[OK] Servo SER0067 attaché sur D0");
    } else {
        servo_ok = false;
        Serial.println("[ERREUR] Impossible d’attacher le SER0067 sur D0");
    }

    // ----- Driver 2x PWM sur D4 / D5 -----
    pinMode(DRIVER_PWM_A, OUTPUT);
    pinMode(DRIVER_PWM_B, OUTPUT);

    analogWrite(DRIVER_PWM_A, 0);
    analogWrite(DRIVER_PWM_B, 0);

    Serial.println("[OK] Driver PWM initialisé sur D4/D5");

    // même si le servo échoue, on ne bloque pas
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

// Vider la ballast : position "seringue sortie / poussée"
void CommandMotor::ballastVider()
{
  if (!servo_ok) {
    return;
  }

  // TODO:
  // - Calculer l’angle correspondant à "ballast vide"
  //   en fonction de la course de la crémaillère et du rapport dents/angle.
  //   Par exemple : angleEmptyDeg = ...
  float angleEmptyDeg = 0.0f; // à ajuster avec ta géométrie

  setServoAngle(angleEmptyDeg);
}

// Remplir la ballast : position "seringue côté ballast"
void CommandMotor::ballastRemplir()
{
  if (!servo_ok) {
    return;
  }

  // TODO:
  // - Calculer l’angle correspondant à "ballast pleine"
  //   (course max de la seringue dans l'autre sens).
  float angleFullDeg = 180.0f; // valeur fictive, à ajuster

  setServoAngle(angleFullDeg);
}

// Ajuster la ballast pour rester à une profondeur cible
void CommandMotor::ballastSuivreProfondeur(float targetDepth_m, float currentDepth_m)
{
  if (!servo_ok) {
    return;
  }

  // Erreur de profondeur ( > 0 : trop haut ou trop bas, selon ta convention)
  float err = targetDepth_m - currentDepth_m;

  // TODO:
  // 1) Convertir l’erreur de profondeur (m) en déplacement linéaire de la crémaillère (mm)
  //    ex: float deltaRack_mm = k_depthToRack * err;
  //
  // 2) Convertir ce déplacement linéaire en angle servo (deg) via l’engrenage
  //    ex: float deltaAngleDeg = k_rackToAngle * deltaRack_mm;
  //
  // 3) Ajouter ce delta à un angle "neutre" correspondant à la flottabilité neutre
  //    ex: float angleCmdDeg = angleNeutralDeg + deltaAngleDeg;
  //
  // Pour l’instant on met juste un placeholder simple (P-control très naïf) :
  
  float Kp_simple = 5.0f; // à ajuster / remplacer par ton calcul géométrique
  float angleCmdDeg = 90.0f + Kp_simple * err;  // 90° = position neutre fictive

  // Sécurisation : clip dans [0 ; 180]
  if (angleCmdDeg < 0.0f)   angleCmdDeg = 0.0f;
  if (angleCmdDeg > 180.0f) angleCmdDeg = 180.0f;

  setServoAngle(angleCmdDeg);
}

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
        // Sens "avant" : D4 actif, D5 à 0
        analogWrite(DRIVER_PWM_A, pwm);
        analogWrite(DRIVER_PWM_B, 0);
    }
    else if (command < 0.0f) {
        // Sens "arrière" : D5 actif, D4 à 0
        analogWrite(DRIVER_PWM_A, 0);
        analogWrite(DRIVER_PWM_B, pwm);
    }
    else {
        // Stop
        analogWrite(DRIVER_PWM_A, 0);
        analogWrite(DRIVER_PWM_B, 0);
    }
}
