#ifndef CAPTEURS_H
#define CAPTEURS_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <INA236.h>     // DVA48 Joy-It
#include <MS5837.h>     // BR-BAR02 / profondeur

// =====================
//   Structures de données
// =====================

struct IMUData
{
    float yaw, pitch, roll;        // Orientation en deg
    float ax, ay, az;              // Accélération linéaire (m/s²)
    float gx, gy, gz;              // Vitesses angulaires (rad/s)

    uint8_t sysCal, gyroCal, accelCal, magCal;  // Calibration
};

struct PowerData
{
    float busVoltage_V;
    float shuntVoltage_mV;
    float current_mA;
    float power_mW;
};

struct LeakData
{
    float humidity_percent;
    float temperature_C;
    bool leakDetected;
};

struct DepthData
{
    float pressure_mbar;
    float depth_m;
    float temperature_C;
};

struct CapteursData
{
    IMUData   imu;
    PowerData power;
    LeakData  leak;
    DepthData depth;
};

// =====================
//   Classe Capteurs
// =====================

class Capteurs
{
public:
    /**
     * Constructeur permettant de définir les adresses I2C des 4 capteurs.
     *
     *  - BNO055  → 0x28 (ADR → GND)
     *  - INA236  → 0x40 (adresse DVA48 par défaut)
     *  - HIH7121 → 0x27
     *  - MS5837  → 0x76 (souvent 0x76 ou 0x77)
     */
    Capteurs(
        uint8_t bnoAddress = 0x28,
        uint8_t inaAddress = 0x40,
        uint8_t hihAddress = 0x27,
        uint8_t msAddress  = 0x76
    );

    bool begin();
    void calibrate(bool verbose = true);
    void update();
    void printDebug();

    const IMUData&      getIMUData()   const { return data.imu; }
    const PowerData&    getPowerData() const { return data.power; }
    const LeakData&     getLeakData()  const { return data.leak; }
    const DepthData&    getDepthData() const { return data.depth; }
    const CapteursData& getAllData()   const { return data; }

private:
    // === Adresses I2C configurables (déclarées AVANT les objets) ===
    uint8_t bno_addr;
    uint8_t ina_addr;
    uint8_t hih_addr;
    uint8_t ms_addr;

    // === Objets capteurs ===
    Adafruit_BNO055 bno;   // IMU
    INA236          ina;   // Courant / tension
    MS5837          baro;  // Profondeur

    // === Flags d'état ===
    bool imu_ok;
    bool ina_ok;
    bool leak_ok;
    bool depth_ok;

    CapteursData data;

    // Lecture brute du HIH7121 (utilise hih_addr)
    bool readHIH7121(float &humidity, float &temperature);
};

#endif
