#ifndef CAPTEURS_H
#define CAPTEURS_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <INA236.h>
#include <MS5837.h>

// =====================
//   Structures de données
// =====================

struct IMUData
{
    float yaw, pitch, roll;
    float ax, ay, az;
    float gx, gy, gz;

    uint8_t sysCal, gyroCal, accelCal, magCal;
};

struct PowerData
{
    // Voie 1 (INA #1, batterie globale)
    float busVoltage_V;
    float shuntVoltage_mV;
    float current_mA;
    float power_mW;

    // Voie 2 (INA #2, mesure)
    float busVoltage2_V;
    float shuntVoltage2_mV;
    float current2_mA;
    float power2_mW;

    // SoC (%) UNIQUEMENT sur la batterie (voie 1)
    float soc1_percent;
};

struct LeakData
{
    // SOS Leak Sensor (BlueRobotics) : sortie digitale
    bool sensorPresent;  // "présence" au boot (test de cohérence du signal)
    bool leakNow;        // état instantané (HIGH = fuite)
    bool leakLatched;    // mémorisé : reste true jusqu'au reset si latch activé
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
//   CoulombCounter
// =====================

class CoulombCounter {
public:
    CoulombCounter(float capacity_mAh = 2200.0f, float initial_soc = 100.0f);

    void reset(float initial_soc = 100.0f);

    // courant en mA, positif = décharge
    void update(float current_mA);

    float get_soc() const;

private:
    float         capacity_mAh;
    float         charge_mAh;
    unsigned long last_millis;
    bool          initialized;
};

// =====================
//   Classe Capteurs
// =====================

class Capteurs
{
public:
    // ⚠️ Signature conservée (hihAddress ignoré) + ajout leakPin/leakLatch en option
    Capteurs(
        uint8_t bnoAddress        = 0x28,
        uint8_t inaBattAddress    = 0x40,  // batterie globale
        uint8_t inaMesureAddress  = 0x41,  // mesure
        uint8_t hihAddress        = 0x27,  // IGNORÉ (ancien capteur humidité)
        uint8_t msAddress         = 0x76,
        float   battCapacity_mAh  = 2200.0f,
        uint8_t leakPin           = 2,     // D2 par défaut
        bool    leakLatch         = true   // mémorise la fuite
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

    // ✅ Ajout pour Safety (évite d'exposer une struct BatteryData inexistante)
    float getBatteryPercent() const;

private:
    uint8_t bno_addr;
    uint8_t ina_batt_addr;     // 0x40
    uint8_t ina_mesure_addr;   // 0x41
    uint8_t hih_addr;          // IGNORÉ (compat)
    uint8_t ms_addr;

    // SOS Leak Sensor (digital)
    uint8_t leak_pin;
    bool    leak_latch;

    Adafruit_BNO055 bno;
    INA236          ina_batt;    // batterie
    INA236          ina_mesure;  // mesure
    MS5837          baro;

    bool imu_ok;
    bool ina_batt_ok;
    bool ina_mesure_ok;
    bool depth_ok;

    // CoulombCounter UNIQUEMENT pour la batterie
    CoulombCounter coulomb_batt;

    CapteursData data;

    // test de cohérence du leak sensor au boot (signal stable / fuite au boot)
    void leakBootCheck(uint32_t test_ms = 500, uint8_t max_transitions = 5);
};

#endif
