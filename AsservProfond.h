// AsservProfond.h
#ifndef ASSERV_PROFOND_H
#define ASSERV_PROFOND_H

// Il faut inclure les headers des dépendances
#include "CommandMotor.h"
#include "Capteurs.h"

class AsservProfond {
public:
    // Constructeur
    AsservProfond(CommandMotor* motorPtr, Capteurs* capteursPtr);

    // Méthode principale appelée depuis le loop()
    void setProfondeurVoulue(float ProfMetres);

private:
    // Pointeurs vers les autres objets
    CommandMotor* _motor;
    Capteurs* _capteurs;

    // --- AJOUTEZ CES VARIABLES ---
    float _gainProportionnel;
    float _angleNeutre;

    // Méthodes internes (helpers)
    float getProfondeur();
    void setServoAngle(float angle);
};

#endif
