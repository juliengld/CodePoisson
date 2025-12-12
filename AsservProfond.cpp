#include "CommandMotor.h"
#include "Capteurs.h"

// Définitions des constantes (à ajuster selon ton robot)
#define PROFONDEUR_MAX 180.0f
#define ANGLE_NEUTRE 180.0f  // Position du servo pour maintenir la profondeur (ou l'horizontale)
#define GAIN_PROPORTIONNEL 10.0f // Facteur de réactivité (Kp)
#define ANGLE_MIN 0.0f
#define ANGLE_MAX 360.0f

void AsservProfond::setProfondeurVoulue(float ProfMetres)
{
    // 1. Sécurité : Bornage de la consigne (ce que tu avais déjà fait)
    if (ProfMetres < 0.0f) ProfMetres = 0.0f;
    if (ProfMetres > PROFONDEUR_MAX) ProfMetres = PROFONDEUR_MAX;

    // 2. Lecture de la profondeur actuelle via les capteurs
    // J'assume que tu as une fonction de ce type dans Capteurs.h
    // Exemple : float ProfActuelle = capteurs.getProfondeur();
    float ProfActuelle = getProfondeur(); 

    // 3. Calcul de l'erreur
    float erreur = ProfMetres - ProfActuelle;

    // 4. Calcul de la commande (Correcteur Proportionnel simple)
    // Si erreur > 0 (on est trop haut), on augmente l'angle pour descendre
    // Si erreur < 0 (on est trop bas), on diminue l'angle pour remonter
    float commandeAngle = ANGLE_NEUTRE + (erreur * GAIN_PROPORTIONNEL);

    // 5. Sécurité : Bornage de la commande pour le servo 360°
    if (commandeAngle < ANGLE_MIN) commandeAngle = ANGLE_MIN;
    if (commandeAngle > ANGLE_MAX) commandeAngle = ANGLE_MAX;

    // 6. Envoi de la commande au moteur
    setServoAngle(commandeAngle);
}
