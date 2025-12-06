#include <Arduino.h>
#include "CommandMotor.h"
#include "Controller.h"
#include "Capteurs.h"
#include "Wifi.h"

// Objets globaux
CommandMotor commandMotor;
Controller   controller(commandMotor);
Capteurs     capteurs;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // attendre ouverture port série (USB natif)
  }

  Serial.println("=== DEMARRAGE POISSON  ===");
  Serial.println("Baud 115200. Tape z/q/s/d/a puis ENTER.");
  Serial.println();

  // ----- Init moteur + controller -----
  Serial.println("[SETUP] Init CommandMotor...");
  commandMotor.begin();

  Serial.println("[SETUP] Init Controller...");
  controller.begin();   // doit afficher [Controller] Initialisation

  // ----- Init capteurs -----
  Serial.println("[SETUP] Init Capteurs...");
  if (!capteurs.begin()) {
    Serial.println("[SETUP] ERREUR capteurs, blocage.");
    while (1) { delay(10); }
  }

  Serial.println("[SETUP] Calibration capteurs...");
  capteurs.calibrate(true);

  // ----- Init Wifi -----
  Serial.println("Init Wifi...");
  setupWifi();

  Serial.println("[SETUP] OK. Pret.");
  Serial.println();
}

void loop() {
  // 1) LECTURE DES TOUCHES SERIE
  while (Serial.available() > 0) {
    char c = Serial.read();
    // On ignore juste les retours ligne
    if (c == '\r' || c == '\n') {
      continue;
    }

    controller.onKey(c);
  }

  // 2) LOGIQUE CONTROLLER (AUTO plus tard)
  controller.update();

  // 3) CAPTEURS (on les met APRES pour ne pas bloquer la lecture série)
  capteurs.update();
  //capteurs.printDebug();

  // 4) Wifi : traitement des requêtes HTTP
  gestionServeurWeb(controller, capteurs);

  // 5A) petite pause pour ne pas saturer le port série
  delay(50);
}
