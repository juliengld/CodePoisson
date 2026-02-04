#include "CommandMotor.h"
#include <Servo.h>

// Variable pour mémoriser l'état : 0 = Centre, 1 = Droite, -1 = Gauche
static int etatDirection = 0; 
static const int DUREE_MOUVEMENT = 1000; // Temps en ms (1 seconde)

CommandMotor::CommandMotor()
{
    servo_ok = false;
    servoDirection_ok = false; 
}

bool CommandMotor::begin()
{
    // ----- Servo ballast sur SERVO_PIN (D3) -----
    // On utilise bien les bornes 500-2500 définies dans le .h
    if (servo.attach(SERVO_PIN, pulseMin_us, pulseMax_us)) {
        servo_ok = true;
        Serial.println("[OK] Servo SER0067 (Ballast) attaché sur D3");
    } else {
        servo_ok = false;
        Serial.println("[ERREUR] Impossible d’attacher le SER0067");
    }

    // ----- Servo direction sur SERVO_DIRECTION_PIN -----
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

    return true;
}

// ============================================================
//   CŒUR DU PROBLÈME RÉSOLU ICI
// ============================================================
void CommandMotor::setServoAngle(float angleDeg)
{
    if (!servo_ok) return;

    // 1. On autorise la plage complète 0-360
    if (angleDeg < 0.0f)   angleDeg = 0.0f;
    if (angleDeg > 360.0f) angleDeg = 360.0f; 

    // 2. On convertit l'angle en microsecondes manuellement
    // Car servo.write(270) ne fonctionne pas toujours sur la librairie standard
    // 0° -> 500us | 360° -> 2500us
    int pulseWidth = map((long)angleDeg, 0, 360, pulseMin_us, pulseMax_us);

    // 3. Envoi précis
    servo.writeMicroseconds(pulseWidth);
}

// ============================================================
//   GESTION BALLAST (Mis à jour pour 360°)
// ============================================================

void CommandMotor::ballastVider()
{
    if (!servo_ok) return;
    // 0° = Seringue vide (piston rentré ou sorti selon montage)
    setServoAngle(0.0f); 
}

void CommandMotor::ballastRemplir()
{
    if (!servo_ok) return;
    // 360° = Seringue pleine (course max)
    setServoAngle(360.0f); 
}

void CommandMotor::ballastEquilibre()
{
    if (!servo_ok) return;
    // Position neutre théorique
    setServoAngle(180.0f); 
}

// ============================================================
//   GESTION SERVO DE DIRECTION (Inchangé)
// ============================================================

void CommandMotor::servoDirectionDroite()
{
    if (etatDirection == 1) return;
    if (etatDirection == -1) servoDirectionStop();

    Serial.println("[Motor] Braquage DROITE...");
    servoDirection.write(0); 
    delay(DUREE_MOUVEMENT);
    servoDirection.write(90); 
    etatDirection = 1; 
}

void CommandMotor::servoDirectionGauche()
{
    if (etatDirection == -1) return;
    if (etatDirection == 1) servoDirectionStop();

    Serial.println("[Motor] Braquage GAUCHE...");
    servoDirection.write(180); 
    delay(DUREE_MOUVEMENT);
    servoDirection.write(90); 
    etatDirection = -1;
}

void CommandMotor::servoDirectionStop()
{
    if (etatDirection == 0) return; 

    Serial.println("[Motor] Retour au CENTRE...");
    if (etatDirection == 1) {
        servoDirection.write(180);
        delay(DUREE_MOUVEMENT);
        servoDirection.write(90);
    }
    else if (etatDirection == -1) {
        servoDirection.write(0);
        delay(DUREE_MOUVEMENT);
        servoDirection.write(90);
    }
    etatDirection = 0;
}

// ============================================================
//   DRIVER 2x PWM (Inchangé)
// ============================================================

void CommandMotor::setDriverRaw(uint8_t pwm4, uint8_t pwm5)
{
    analogWrite(DRIVER_PWM_A, pwm4);
    analogWrite(DRIVER_PWM_B, pwm5);
}

void CommandMotor::setDriverCommand(float command)
{
    if (command < 0.0f) command = 0.0f;
    if (command > 1.0f) command = 1.0f;
    uint8_t pwm = (uint8_t)(command * 255.0f + 0.5f);
    setDriverRaw(pwm, 0);
}