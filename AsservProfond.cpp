#include CommandMotor.h
#include Capteurs.h

  void AsservProfond::setProfondeurVoulue(float ProfMetres)
{

    
    if (ProfMetres < 0.0f)   ProfMetres = 0.0f;
    if (ProfMetres > 180.0f) ProfMetres = 180.0f;

    servo.write(angleDeg);
}

setServoAngle
