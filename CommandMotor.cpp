#include "CommandMotor.h"

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

void CommandMotor::setDriverRaw(uint8_t pwmD4, uint8_t pwmD5)
{
    analogWrite(DRIVER_PWM_A, pwmD4);
    analogWrite(DRIVER_PWM_B, pwmD5);
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
