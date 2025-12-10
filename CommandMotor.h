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

    void setDriverCommand(float command);

    // === GESTION BALLAST PAR SERVO ===
    void ballastVider();                               // vider la ballast
    void ballastRemplir();                             // remplir la ballast
    void ballastSuivreProfondeur(float targetDepth_m, float currentDepth_m);

    // === GESTION SERVO DE DIRECTION (2e servo) ===
    // Tourner le poisson / la queue à droite
    void servoDirectionDroite();

    // Tourner le poisson / la queue à gauche
    void servoDirectionGauche();

private:
    // -------- SERVO BALLAST --------
    Servo servo;
    bool  servo_ok;

    static const int SERVO_PIN     = 3;
    static const int pulseMin_us   = 500;   // SER0067
    static const int pulseMax_us   = 2500;  // SER0067

    // -------- SERVO DIRECTION --------
    Servo servoDirection;          // 2e servomoteur pour tourner droite/gauche
    bool  servoDirection_ok = false;

    // À ajuster suivant ton câblage réel
    static const int SERVO_DIRECTION_PIN = 6;

    // -------- DRIVER 2x PWM --------
    static const int DRIVER_PWM_A  = 4;
    static const int DRIVER_PWM_B  = 5;
};

#endif
