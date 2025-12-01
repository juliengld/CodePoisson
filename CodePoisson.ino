#include "Capteurs.h"

Capteurs capteurs;

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  if (!capteurs.begin()) {
    Serial.println("Erreur init capteurs, blocage.");
    while (1);
  }

  capteurs.calibrate(true);  // tu peux mettre false pour moins de 
  
  Serial.println("Init CommandMotor...");
  commandMotor.begin();
}

void loop() {
  capteurs.update();      // lit IMU + courant/tension
  capteurs.printDebug();  // affiche tout proprement

  delay(100);             // fréquence d’affichage ~10 Hz
}
