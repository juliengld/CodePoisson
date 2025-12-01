#ifndef COMMAND_MOTOR_H
#define COMMAND_MOTOR_H

#include <Arduino.h>
#include <Servo.h>

class CommandMotor
{
public:
    CommandMotor();

    // Initialise le servo sur D0 et le driver sur D4/D5
    bool begin();

    // ------- SERVO (SER0067 Feetech sur D0) -------
    // Commande l’angle du servo en degrés [0 ; 180]
    void setServoAngle(float angleDeg);

    // ------- DRIVER 2x PWM (D4 / D5) -------
    // Commande brute : valeurs PWM 0–255 pour chaque pin
    void setDriverRaw(uint8_t pwmD4, uint8_t pwmD5);

    // Commande normalisée : -1.0 → -100% / 0 → stop / +1.0 → +100%
    // Un seul sens actif à la fois :
    //   >0 => D4 PWM, D5 = 0
    //   <0 => D4 = 0,  D5 PWM
    void setDriverCommand(float command);

private:
    // -------- SERVO --------
    Servo servo;
    bool  servo_ok;

    static const int SERVO_PIN     = D0;
    static const int pulseMin_us   = 500;   // SER0067
    static const int pulseMax_us   = 2500;  // SER0067

    // -------- DRIVER 2x PWM --------
    static const int DRIVER_PWM_A  = D4;
    static const int DRIVER_PWM_B  = D5;
};

#endif
