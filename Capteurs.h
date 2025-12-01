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
    Capteurs();

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
    // Capteurs
    Adafruit_BNO055 bno;
    INA236          ina;
    MS5837          baro;

    bool imu_ok;
    bool ina_ok;
    bool leak_ok;
    bool depth_ok;

    // HIH7121 : pas de librairie → adresse I2C = 0x27
    const uint8_t HIH_ADDR = 0x27;

    CapteursData data;

    // Lecture brute du HIH7121
    bool readHIH7121(float &humidity, float &temperature);
};

#endif
