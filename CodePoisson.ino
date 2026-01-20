#include <Arduino.h>
#include "CommandMotor.h"
#include "Controller.h"
#include "Capteurs.h"
#include "Wifi.h"
#include "Safety.h"
#include "StateMachine.h"

// ==========================================
// INSTANCIATION DES OBJETS GLOBAUX
// ==========================================

CommandMotor commandMotor;

Safety safety;

// Instanciation détaillée des capteurs (issue de ta version "Upstream")
Capteurs capteurs(
  0x28,    // BNO055
  0x40,    // INA batterie (SoC)
  0x41,    // INA mesure
  0x27,    // HIH
  0x76,    // MS5837
  2200.0f  // capacité batterie (mAh)
);

// StateMachine a besoin de Motor, Capteurs et Safety
StateMachine stateMachine(commandMotor, capteurs, safety);

// Controller a besoin de Motor et StateMachine
Controller controller(commandMotor, stateMachine);


// ==========================================
// SETUP
// ==========================================
void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // attendre ouverture port série (USB natif)
  }

  Serial.println("=== DEMARRAGE POISSON  ===");
  Serial.println("Baud 115200. Tape z/q/s/d/a puis ENTER.");
  Serial.println();

  // 1. Init Moteur
  Serial.println("[SETUP] Init CommandMotor...");
  commandMotor.begin();

  // 2. Init Controller
  Serial.println("[SETUP] Init Controller...");
  controller.begin();

  // 3. Init Capteurs
  Serial.println("[SETUP] Init Capteurs...");
  if (!capteurs.begin()) {
    Serial.println("[SETUP] ERREUR capteurs, blocage.");
    // On ne bloque pas infiniment pour le debug, mais attention en réel
    // while (1) { delay(10); } 
  }

  Serial.println("[SETUP] Calibration capteurs...");
  capteurs.calibrate(true);

  // 4. Init Safety & StateMachine
  safety.begin();
  stateMachine.begin();

  // 5. Init Wifi
  Serial.println("Init Wifi...");
  setupWifi();

  Serial.println("[SETUP] OK. Pret.");
  Serial.println();
}

// ==========================================
// LOOP
// ==========================================
void loop() {
  
  // 1) LECTURE DES TOUCHES SERIE (Tout au même endroit)
  while (Serial.available() > 0) {
    char c = Serial.read();
    
    // On ignore les retours ligne
    if (c == '\r' || c == '\n') continue;

    // Test d'urgence manuel
    if (c == 'e') {
      Serial.println("!!! EMERGENCY STATE TRIGGERED MANUALLY !!!");
      stateMachine.setEmergency(EmergencyState::LEAK);
    } 
    else {
      // Sinon on envoie la touche au controller
      controller.onKey(c);
    }
  }

  // 2) LOGIQUE PRINCIPALE
  
  // Met à jour le mode (Manuel/Auto)
  // Note: Controller appelle stateMachine.update() SI on est en mode AUTONOMOUS
  controller.update();

  // 3) CAPTEURS & SAFETY
  capteurs.update();
  
  // Safety check (retourne un état d'urgence si problème détecté)
  EmergencyState e = safety.update(capteurs);
  
  // Si le Safety détecte un problème, on force l'urgence dans la machine
  if (e != EmergencyState::NONE) {
      stateMachine.setEmergency(e);
  }

  // 4) STATE MACHINE (Mise à jour inconditionnelle pour gérer l'urgence)
  // On l'appelle ici pour être sûr que l'état EMERGENCY est géré même en mode MANUEL
  // (La fonction update() du StateMachine a une protection pour ne rien faire si IDLE)
  stateMachine.update();

  // 5) WIFI
  gestionServeurWeb(controller, capteurs);

  // Petite pause pour ne pas saturer
  delay(50);
}