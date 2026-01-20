#include <Arduino.h>
#include "CommandMotor.h"
#include "Controller.h"
#include "Capteurs.h"
#include "Wifi.h"
#include "Safety.h"
#include "StateMachine.h"

// Objets globaux
CommandMotor commandMotor;
Safety safety;
Controller   controller(commandMotor);
Capteurs capteurs(
  0x28,    // BNO055
  0x40,    // INA batterie (SoC)
  0x41,    // INA mesure
  0x27,    // HIH
  0x76,    // MS5837
  2200.0f  // capacité batterie (mAh)
);
StateMachine stateMachine(commandMotor, capteurs, safety);


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

  safety.begin();

  // ----- Init Wifi -----
  Serial.println("Init Wifi...");
  setupWifi();

  stateMachine.begin();

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

  EmergencyState e = safety.update(capteurs);

  // 4) Wifi : traitement des requêtes HTTP
  gestionServeurWeb(controller, capteurs);

   // ---- TEST EMERGENCY ----
  if (Serial.available()) {
    char c = Serial.read();

    if (c == 'e') {
      Serial.println("!!! EMERGENCY STATE TRIGGERED !!!");
      stateMachine.setEmergency(EmergencyState::LEAK);
    }
  }

  stateMachine.update();

  // 5A) petite pause pour ne pas saturer le port série
  delay(50);
}
