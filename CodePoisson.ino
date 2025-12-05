#include "Capteurs.h"
#include "CommandMotor.h"
#include "Controller.h"
#include "Wifi.h"  // On ajoute le Wifi

Capteurs capteurs;
CommandMotor commandMotor;
// Le contrôleur a besoin du moteur, on lui passe à la construction
Controller controller(commandMotor);

void setup() {
    Serial.begin(115200);
    // while (!Serial) {} // Décommentez pour debug, commentez pour autonomie

    // 1. Init Capteurs
    if (!capteurs.begin()) {
        Serial.println("Erreur init capteurs, blocage.");
        while (1);
    }
    capteurs.calibrate(true); 

    // 2. Init Moteurs
    Serial.println("Init CommandMotor...");
    commandMotor.begin();
    
    // 3. Init Controller
    controller.begin();

    // 4. Init Wifi
    setupWifi();
}

void loop() {
    // A. Mise à jour des capteurs
    capteurs.update();
    
    // B. Gestion des commandes Wifi
    // On passe 'controller' et 'capteurs' pour que le Wifi puisse les utiliser
    gestionServeurWeb(controller, capteurs);

    // C. Mise à jour de la logique robot (autonome ou manuel)
    controller.update();

    // Pas de delay() trop long ici sinon le Wifi lag
    // Si tu veux un debug capteur, utilise un timer millis()
    /*
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 1000) {
       capteurs.printDebug();
       lastPrint = millis();
    }
    */
}