#include "Capteurs.h"
#include <string.h>

static const int32_t BNO_SENSOR_ID = 55;

// =====================
//   Constructeur
// =====================

Capteurs::Capteurs(uint8_t bnoAddress,
                   uint8_t inaAddress,
                   uint8_t hihAddress,
                   uint8_t msAddress)
: bno_addr(bnoAddress)
, ina_addr(inaAddress)
, hih_addr(hihAddress)
, ms_addr(msAddress)
, bno(BNO_SENSOR_ID, bno_addr, &Wire)   // Adafruit_BNO055(id, addr, Wire)
, ina(ina_addr, &Wire)                  // INA236(addr, Wire)
, baro()                                // MS5837 : constructeur par défaut
{
    memset(&data, 0, sizeof(CapteursData));

    imu_ok   = false;
    ina_ok   = false;
    leak_ok  = true;    // HIH7121 : lecture I2C juste false en cas d'erreur
    depth_ok = false;
}

// =====================
//   Initialisation
// =====================

bool Capteurs::begin()
{
    Wire.begin();

    // ===== IMU =====
    if (bno.begin(OPERATION_MODE_NDOF)) {
        imu_ok = true;
        delay(1000);
        bno.setExtCrystalUse(true);
        Serial.println("[OK] IMU BNO055 détectée");
    } else {
        imu_ok = false;
        Serial.println("[ERREUR] IMU BNO055 non détectée (ignorée)");
    }

    // ===== INA236 courant/tension =====
    if (ina.begin()) {
        ina_ok = true;
        Serial.println("[OK] INA236 détecté");
    } else {
        ina_ok = false;
        Serial.println("[ERREUR] INA236 non détecté (ignoré)");
    }

    // ===== MS5837 profondeur =====
    if (baro.init()) {
        depth_ok = true;
        baro.setModel(MS5837::MS5837_02BA);
        baro.setFluidDensity(997);
        Serial.println("[OK] Capteur de profondeur MS5837 détecté");
    } else {
        depth_ok = false;
        Serial.println("[ERREUR] MS5837 non détecté (ignoré)");
    }

    // toujours true : aucune init ne bloque
    return true;
}

// =====================
//   Calibration IMU
// =====================

void Capteurs::calibrate(bool verbose)
{
    if (!imu_ok) {
        Serial.println("[INFO] IMU absente → calibration ignorée");
        return;
    }

    // Lecture unique de l'état de calibration
    uint8_t sys = 0, gyro = 0, accel = 0, mag = 0;
    bno.getCalibration(&sys, &gyro, &accel, &mag);

    data.imu.sysCal   = sys;
    data.imu.gyroCal  = gyro;
    data.imu.accelCal = accel;
    data.imu.magCal   = mag;

    // Si tu veux garder un tout petit retour :
    //if (verbose) {
        //Serial.print("Calibration [sys,g,a,m] = [");
        //Serial.print(sys);   Serial.print(", ");
        //Serial.print(gyro);  Serial.print(", ");
        //Serial.print(accel); Serial.print(", ");
        //Serial.print(mag);   Serial.println("]");
    //}

    if (sys == 3) {
        Serial.println("Calibration terminée");
    }
}


// =====================
//   Mise à jour capteurs
// =====================

void Capteurs::update()
{
    // ===== IMU =====
    if (imu_ok)
    {
        sensors_event_t euler;
        bno.getEvent(&euler, Adafruit_BNO055::VECTOR_EULER);
        data.imu.yaw   = euler.orientation.x;
        data.imu.roll  = euler.orientation.y;
        data.imu.pitch = euler.orientation.z;

        sensors_event_t acc;
        bno.getEvent(&acc, Adafruit_BNO055::VECTOR_ACCELEROMETER);
        data.imu.ax = acc.acceleration.x;
        data.imu.ay = acc.acceleration.y;
        data.imu.az = acc.acceleration.z;

        sensors_event_t gyro;
        bno.getEvent(&gyro, Adafruit_BNO055::VECTOR_GYROSCOPE);
        data.imu.gx = gyro.gyro.x;
        data.imu.gy = gyro.gyro.y;
        data.imu.gz = gyro.gyro.z;

        bno.getCalibration(
            &data.imu.sysCal,
            &data.imu.gyroCal,
            &data.imu.accelCal,
            &data.imu.magCal
        );
    }

    // ===== COURANT / TENSION =====
    if (ina_ok)
    {
        data.power.busVoltage_V    = ina.getBusVoltage();
        data.power.shuntVoltage_mV = ina.getShuntVoltage();
        data.power.current_mA      = ina.getCurrent();
        data.power.power_mW        = ina.getPower();
    }

    // ===== PROFONDEUR =====
    if (depth_ok)
    {
        baro.read();
        data.depth.pressure_mbar  = baro.pressure();
        data.depth.temperature_C  = baro.temperature();
        data.depth.depth_m        = baro.depth();
    }
}

// =====================
//   Affichage debug
// =====================

void Capteurs::printDebug()
{
    Serial.println("====================================");

    // ---- IMU ----
    if (imu_ok) {
        Serial.println("IMU :");
        Serial.print("  Yaw / Pitch / Roll [deg] : ");
        Serial.print(data.imu.yaw);   Serial.print(" / ");
        Serial.print(data.imu.pitch); Serial.print(" / ");
        Serial.println(data.imu.roll);

        Serial.print("  Acc [m/s^2] : ");
        Serial.print(data.imu.ax); Serial.print(" / ");
        Serial.print(data.imu.ay); Serial.print(" / ");
        Serial.println(data.imu.az);

        Serial.print("  Gyro [rad/s] : ");
        Serial.print(data.imu.gx); Serial.print(" / ");
        Serial.print(data.imu.gy); Serial.print(" / ");
        Serial.println(data.imu.gz);
    } else {
        Serial.println("IMU absente");
    }

    // ---- INA ----
    if (ina_ok) {
        Serial.print("INA236: ");
        Serial.print(data.power.busVoltage_V);
        Serial.print(" V, ");
        Serial.print(data.power.current_mA);
        Serial.println(" mA");
    } else {
        Serial.println("INA236 absent");
    }

    // ---- Profondeur ----
    if (depth_ok) {
        Serial.print("Profondeur: ");
        Serial.print(data.depth.depth_m);
        Serial.println(" m");
    } else {
        Serial.println("Capteur profondeur absent");
    }

    Serial.println("====================================");
}
