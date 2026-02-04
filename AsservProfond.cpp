#include "AsservProfond.h"

AsservProfond::AsservProfond(CommandMotor* motorPtr, Capteurs* capteursPtr) {
    _motor = motorPtr;
    _capteurs = capteursPtr;
    
    // Gain: A ajuster. 
    // Comme la course est grande (360°), on peut augmenter le gain par rapport à un servo 180°.
    _gainProportionnel = 40.0f; 
    
    // Neutre: Milieu de la course du servo 360
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
    // 1. Sécurité borne consigne
    if (ProfMetres < 0.0f) ProfMetres = 0.0f;
    if (ProfMetres > _profondeurSecurite) ProfMetres = _profondeurSecurite;

    // 2. Lecture
    float ProfActuelle = getProfondeur(); 

    // 3. Calcul Erreur 
    // Si ProfActuelle < Cible (on est trop haut) -> Erreur > 0 -> On doit remplir (augmenter angle)
    float erreur = ProfMetres - ProfActuelle;

    // 4. Commande P
    float commandeAngle = _angleNeutre + (erreur * _gainProportionnel);

    // 5. Saturation 0-360
    if (commandeAngle < _angleMin) commandeAngle = _angleMin;
    if (commandeAngle > _angleMax) commandeAngle = _angleMax;

    // 6. Envoi
    setServoAngle(commandeAngle);
    
    // Debug optionnel
    /*
    Serial.print("Cible: "); Serial.print(ProfMetres);
    Serial.print(" Actuel: "); Serial.print(ProfActuelle);
    Serial.print(" Cmd: "); Serial.println(commandeAngle);
    */
}