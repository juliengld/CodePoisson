#include "CommandMotor.h"
#include "Capteurs.h"
#include "AsservProfond.h"

AsservProfond::AsservProfond(CommandMotor* motorPtr, Capteurs* capteursPtr) {
    _motor = motorPtr;
    _capteurs = capteursPtr;
    
    // Le gain doit être positif si (Angle ++ => On descend)
    _gainProportionnel = 30.0f; // Augmenté car on travaille sur une plage réduite
    _angleNeutre = 30.0f;       // Correspond à ballastEquilibre()
}

float AsservProfond::getProfondeur() {
    return _capteurs->getDepthData().depth_m; 
}

void AsservProfond::setServoAngle(float angle) {
    _motor->setServoAngle(angle); 
}

// Limites physiques du servo (selon CommandMotor.cpp)
#define ANGLE_MIN 0.0f
#define ANGLE_MAX 360.0f 
#define PROFONDEUR_MAX 10.0f // Sécurité : n'essayez pas d'aller trop profond

void AsservProfond::setProfondeurVoulue(float ProfMetres)
{
    // 1. Sécurité Bornage Consigne
    if (ProfMetres < 0.0f) ProfMetres = 0.0f;
    if (ProfMetres > PROFONDEUR_MAX) ProfMetres = PROFONDEUR_MAX;

    // 2. Lecture
    float ProfActuelle = getProfondeur(); 

    // 3. Calcul Erreur (Consigne - Mesure)
    // Ex: Veut 5m, est à 2m => Erreur = 3m (doit descendre)
    float erreur = ProfMetres - ProfActuelle;

    // 4. Commande P (Proportionnelle)
    // AngleNeutre (30°) + (3m * Gain)
    // Si on doit descendre (erreur > 0), on ajoute de l'angle (vers 180 = Remplir) -> CORRECT
    // Si on doit monter (erreur < 0), on enlève de l'angle (vers 0 = Vider) -> CORRECT
    float commandeAngle = _angleNeutre + (erreur * _gainProportionnel);

    // 5. Bornage final
    if (commandeAngle < ANGLE_MIN) commandeAngle = ANGLE_MIN;
    if (commandeAngle > ANGLE_MAX) commandeAngle = ANGLE_MAX;

    // 6. Envoi
    setServoAngle(commandeAngle);
    
    // Debug optionnel pour voir ce qui se passe
    /*
    Serial.print("Cible: "); Serial.print(ProfMetres);
    Serial.print(" Actuel: "); Serial.print(ProfActuelle);
    Serial.print(" Err: "); Serial.print(erreur);
    Serial.print(" Cmd: "); Serial.println(commandeAngle);
    */
}
