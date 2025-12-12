#ifndef ASSERV_PROFOND_H
#define ASSERV_PROFOND_H

// Inclusion des dépendances nécessaires
#include "CommandMotor.h"
#include "Capteurs.h"
#include <Arduino.h> // Souvent nécessaire pour les types comme 'byte' ou 'float' sur microcontrôleur

class AsservProfond {
public:
    /**
     * Constructeur
     * @param motorPtr : Pointeur vers l'objet de commande moteur
     * @param capteursPtr : Pointeur vers l'objet de gestion des capteurs
     */
    AsservProfond(CommandMotor* motorPtr, Capteurs* capteursPtr);

    /**
     * Méthode principale d'asservissement.
     * Calcule l'erreur et envoie la commande au servo.
     * @param ProfMetres : La profondeur cible en mètres.
     */
    void setProfondeurVoulue(float ProfMetres);

    // --- Setters pour le réglage dynamique (optionnel mais recommandé) ---
    
    // Pour changer le gain Kp sans re-téléverser le code
    void setGainProportionnel(float kp); 
    
    // Pour ajuster le "zéro" du servo
    void setAngleNeutre(float angle);

private:
    // --- Objets dépendants ---
    CommandMotor* _motor;
    Capteurs* _capteurs;

    // --- Paramètres de l'asservissement ---
    float _gainProportionnel; // Kp
    float _angleNeutre;       // Angle pour maintenir la position (ex: 180°)
    
    // --- Constantes de sécurité ---
    // (Peuvent être statiques si elles ne changent jamais pour aucune instance)
    const float _profondeurMax = 180.0f;
    const float _angleMin = 0.0f;
    const float _angleMax = 360.0f;

    // --- Méthodes internes ---
    // Abstractions pour simplifier le code principal
    float getProfondeur();
    void setServoAngle(float angle);
};

#endif // ASSERV_PROFOND_H
