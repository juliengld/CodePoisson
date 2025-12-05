#ifndef WIFI_H
#define WIFI_H

#include <WiFiNINA.h>
#include "Controller.h" // Pour connaître le type Controller
#include "Capteurs.h"   // Pour connaître le type Capteurs

// Initialisation du Wifi (connexion box)
void setupWifi();

// Fonction principale à appeler dans le loop()
// On lui passe les références (&) vers le contrôleur et les capteurs
// pour qu'il puisse les lire et les commander.
void gestionServeurWeb(Controller &controller, Capteurs &capteurs);

void printWifiStatus();

#endif