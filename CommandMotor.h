#ifndef COMMAND_MOTOR_H
#define COMMAND_MOTOR_H

#include <Arduino.h>
#include <Servo.h>

class CommandMotor


{
public:
    CommandMotor();

    bool begin();

    // ------- SERVO (SER0067 Feetech sur D0) -------
    void setServoAngle(float angleDeg);

    // ------- DRIVER 2x PWM (D4 / D5) -------
    void setDriverRaw(uint8_t pwmD4, uint8_t pwmD5);

    void setDriverCommand(float command);

    void ballastVider();                               
    void ballastRemplir();                             
    void ballastEquilibre();
    void ballastSuivreProfondeur(float targetDepth_m, float currentDepth_m);

    void servoDirectionDroite();

    void servoDirectionGauche();
    void servoDirectionStop();

private:
    Servo servo;
    bool  servo_ok;

    static const int SERVO_PIN     = 3;
    static const int pulseMin_us   = 500;   // SER0067
    static const int pulseMax_us   = 2500;  // SER0067

    Servo servoDirection;          
    bool  servoDirection_ok = false;

    static const int SERVO_DIRECTION_PIN = 6;

    static const int DRIVER_PWM_A  = 4;
    static const int DRIVER_PWM_B  = 5;
};

#endif
