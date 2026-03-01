#include "AsservProfond.h"

AsservProfond::AsservProfond(CommandMotor* motorPtr, Capteurs* capteursPtr) {
    _motor = motorPtr;
    _capteurs = capteursPtr;
    
    _gainProportionnel = 40.0f; 
    
    _angleNeutre = 180.0f;       
}

void AsservProfond::setGainProportionnel(float kp) {
    _gainProportionnel = kp;
}

void AsservProfond::setAngleNeutre(float angle) {
    _angleNeutre = angle;
}

float AsservProfond::getProfondeur() {
    return _capteurs->getDepthData().depth_m; 
}

void AsservProfond::setServoAngle(float angle) {
    _motor->setServoAngle(angle); 
}

void AsservProfond::setProfondeurVoulue(float ProfMetres)
{
    if (ProfMetres < 0.0f) ProfMetres = 0.0f;
    if (ProfMetres > _profondeurSecurite) ProfMetres = _profondeurSecurite;

    float ProfActuelle = getProfondeur(); 

    float erreur = ProfMetres - ProfActuelle;

    float commandeAngle = _angleNeutre + (erreur * _gainProportionnel);

    if (commandeAngle < _angleMin) commandeAngle = _angleMin;
    if (commandeAngle > _angleMax) commandeAngle = _angleMax;

    setServoAngle(commandeAngle);
    
    // Debug 
    /*
    Serial.print("Cible: "); Serial.print(ProfMetres);
    Serial.print(" Actuel: "); Serial.print(ProfActuelle);
    Serial.print(" Cmd: "); Serial.println(commandeAngle);
    */
}