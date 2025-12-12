#include "CommandMotor.h"
#include <Servo.h>

// Variable pour mémoriser l'état : 0 = Centre, 1 = Droite, -1 = Gauche
static int etatDirection = 0; 
static const int DUREE_MOUVEMENT = 3000; // Temps en ms (1 seconde)

CommandMotor::CommandMotor()
{
    servo_ok = false;
    servoDirection_ok = false; // 2e servo non initialisé par défaut
}

bool CommandMotor::begin()
{
    // ----- Servo ballast sur D0 -----
    if (servo.attach(SERVO_PIN, pulseMin_us, pulseMax_us)) {
        servo_ok = true;
        Serial.println("[OK] Servo SER0067 attaché sur D0");
    } else {
        servo_ok = false;
        Serial.println("[ERREUR] Impossible d’attacher le SER0067 sur D0");
    }

    // ----- Servo direction sur SERVO_DIRECTION_PIN -----
    // NOTE : seulement la structure ici, tu pourras compléter la logique plus tard
    if (servoDirection.attach(SERVO_DIRECTION_PIN, pulseMin_us, pulseMax_us)) {
        servoDirection_ok = true;
        Serial.println("[OK] Servo direction attaché");
    } else {
        servoDirection_ok = false;
        Serial.println("[ERREUR] Impossible d’attacher le servo direction");
    }

    // ----- Driver 2x PWM sur D4 / D5 -----
    pinMode(DRIVER_PWM_A, OUTPUT);
    pinMode(DRIVER_PWM_B, OUTPUT);

    analogWrite(DRIVER_PWM_A, 0);
    analogWrite(DRIVER_PWM_B, 0);

    Serial.println("[OK] Driver PWM initialisé sur D4/D5");

    // même si les servos échouent, on ne bloque pas
    return true;
}

void CommandMotor::setServoAngle(float angleDeg)
{
    if (!servo_ok) return;

    if (angleDeg < 0.0f)   angleDeg = 0.0f;
    if (angleDeg > 180.0f) angleDeg = 180.0f;

    servo.write(angleDeg);
}

// ============================================================
//   GESTION BALLAST PAR SERVO + CREMAILLERE
// ============================================================

void CommandMotor::ballastVider()
{
    if (!servo_ok) {
        return;
    }

    float angleEmptyDeg = 0.0f; // à ajuster avec ta géométrie
    setServoAngle(angleEmptyDeg);
}

void CommandMotor::ballastRemplir()
{
    if (!servo_ok) {
        return;
    }

    float angleFullDeg = 180.0f; // valeur fictive, à ajuster
    setServoAngle(angleFullDeg);
}
void CommandMotor::ballastEquilibre()
{
    if (!servo_ok) {
        return;
    }

    float angleFullDeg = 30.0f; // valeur fictive, à ajuster
    setServoAngle(angleFullDeg);
}

// ============================================================
//   GESTION SERVO DE DIRECTION (2e servo) – SQUELETTE SEULEMENT
// ============================================================

// ... (Haut du fichier inchangé) ...



void CommandMotor::servoDirectionDroite()
{
    // Si on est déjà braqué à droite, on ne fait rien (on attend que la touche soit relâchée)
    if (etatDirection == 1) return;

    // Si on était à gauche, on revient d'abord au centre (sécurité optionnelle)
    if (etatDirection == -1) servoDirectionStop();

    Serial.println("[Motor] Braquage DROITE en cours...");
    
    // 1. On tourne le FT90R (Vitesse sens horaire)
    servoDirection.write(0); 
    
    // 2. On attend le temps qu'il fasse le mouvement
    delay(DUREE_MOUVEMENT);
    
    // 3. On coupe le moteur (il arrête de forcer, mais la crémaillère est en position)
    servoDirection.write(90); 
    
    // 4. On note l'état
    etatDirection = 1; 
    //Serial.println("[Motor] Braquage DROITE terminé (attente).");
}

void CommandMotor::servoDirectionGauche()
{
    // Si on est déjà braqué à gauche, on ne fait rien
    if (etatDirection == -1) return;

    // Si on était à droite, on revient d'abord au centre
    if (etatDirection == 1) servoDirectionStop();

    Serial.println("[Motor] Braquage GAUCHE en cours...");
    
    // 1. On tourne le FT90R (Vitesse sens anti-horaire)
    servoDirection.write(180); 
    
    // 2. On attend
    delay(DUREE_MOUVEMENT);
    
    // 3. On coupe
    servoDirection.write(90); 
    
    // 4. On note l'état
    etatDirection = -1;
    //Serial.println("[Motor] Braquage GAUCHE terminé (attente).");
}

// Fonction pour revenir au centre (à ajouter)
void CommandMotor::servoDirectionStop()
{
    if (etatDirection == 0) return; // Déjà au centre

    Serial.println("[Motor] Retour au CENTRE...");

    if (etatDirection == 1) {
        // On était à Droite, on tourne à Gauche pour revenir
        servoDirection.write(180);
        delay(DUREE_MOUVEMENT);
        servoDirection.write(90);
    }
    else if (etatDirection == -1) {
        // On était à Gauche, on tourne à Droite pour revenir
        servoDirection.write(0);
        delay(DUREE_MOUVEMENT);
        servoDirection.write(90);
    }

    // On est revenu au centre
    etatDirection = 0;
    Serial.println("[Motor] Retour CENTRE terminé.");
}

// ============================================================
//   DRIVER 2x PWM
// ============================================================

void CommandMotor::setDriverRaw(uint8_t pwm4, uint8_t pwm5)
{
    analogWrite(DRIVER_PWM_A, pwm4);
    analogWrite(DRIVER_PWM_B, pwm5);
}

void CommandMotor::setDriverCommand(float command)
{
    // Commande normalisée [0 ; 1]
    if (command < 0.0f) command = 0.0f;
    if (command > 1.0f) command = 1.0f;

    // Conversion en PWM 0–255
    uint8_t pwm = (uint8_t)(command * 255.0f + 0.5f);

    // Moteur UNIQUEMENT en marche avant : D4 = PWM, D5 = 0
    setDriverRaw(pwm, 0);
}
