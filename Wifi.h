#ifndef WIFI_H
#define WIFI_H

#include <WiFiNINA.h>
#include "Controller.h"
#include "Capteurs.h"

void setupWifi();
void gestionServeurWeb(Controller &controller, Capteurs &capteurs);
void printWifiStatus();

#endif
