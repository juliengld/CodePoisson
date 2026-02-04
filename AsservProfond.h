#ifndef ASSERV_PROFOND_H
#define ASSERV_PROFOND_H

#include "CommandMotor.h"
#include "Capteurs.h"
#include <Arduino.h>

class AsservProfond {
public:
    AsservProfond(CommandMotor* motorPtr, Capteurs* capteursPtr);

    void setProfondeurVoulue(float ProfMetres);

    // Setters pour tuning
    void setGainProportionnel(float kp); 
    void setAngleNeutre(float angle);

private:
    CommandMotor* _motor;
    Capteurs* _capteurs;

    float _gainProportionnel;
    float _angleNeutre;
    
    // --- Configuration Servo 360 ---
    const float _angleMin = 0.0f;
    const float _angleMax = 360.0f; 
    const float _profondeurSecurite = 5.0f; // max 5 mètres

    float getProfondeur();
    void setServoAngle(float angle);
};

#endif