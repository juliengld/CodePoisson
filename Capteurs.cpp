#include "Capteurs.h"
#include <Arduino.h>
#include <Wire.h>
#include <string.h>

static const int32_t BNO_SENSOR_ID = 55;

// =====================
//   CoulombCounter
// =====================

CoulombCounter::CoulombCounter(float capacity_mAh, float initial_soc)
    : capacity_mAh(capacity_mAh),
      charge_mAh(capacity_mAh * (initial_soc / 100.0f)),
      last_millis(0),
      initialized(false)
{
}

void CoulombCounter::reset(float initial_soc)
{
    charge_mAh  = capacity_mAh * (initial_soc / 100.0f);
    last_millis = millis();
    initialized = true;
}

void CoulombCounter::update(float current_mA)
{
    unsigned long now = millis();

    if (!initialized) {
        last_millis = now;
        initialized = true;
        return;
    }

    float dt_s = (now - last_millis) / 1000.0f;
    last_millis = now;

    float consumed_mAh = current_mA * (dt_s / 3600.0f);
    charge_mAh -= consumed_mAh;

    if (charge_mAh < 0.0f)         charge_mAh = 0.0f;
    if (charge_mAh > capacity_mAh) charge_mAh = capacity_mAh;
}

float CoulombCounter::get_soc() const
{
    if (capacity_mAh <= 0.0f) return 0.0f;
    return 100.0f * (charge_mAh / capacity_mAh);
}

// =====================
//   Constructeur
// =====================

Capteurs::Capteurs(uint8_t bnoAddress,
                   uint8_t inaBattAddress,
                   uint8_t inaMesureAddress,
                   uint8_t hihAddress,
                   uint8_t msAddress,
                   float   battCapacity_mAh,
                   uint8_t leakPin,
                   bool    leakLatch)
: bno_addr(bnoAddress)
, ina_batt_addr(inaBattAddress)
, ina_mesure_addr(inaMesureAddress)
, hih_addr(hihAddress) // ignoré (compat)
, ms_addr(msAddress)
, leak_pin(leakPin)
, leak_latch(leakLatch)
, bno(BNO_SENSOR_ID, bno_addr, &Wire)
, ina_batt(ina_batt_addr, &Wire)
, ina_mesure(ina_mesure_addr, &Wire)
, baro()
, coulomb_batt(battCapacity_mAh)
{
    memset(&data, 0, sizeof(CapteursData));

    imu_ok        = false;
    ina_batt_ok   = false;
    ina_mesure_ok = false;
    depth_ok      = false;

    data.leak.sensorPresent = false;
    data.leak.leakNow = false;
    data.leak.leakLatched = false;
}

// =====================
//   Leak boot check
// =====================

void Capteurs::leakBootCheck(uint32_t test_ms, uint8_t max_transitions)
{
    uint32_t t0 = millis();
    uint8_t transitions = 0;
    int last = digitalRead(leak_pin);

    while (millis() - t0 < test_ms) {
        int v = digitalRead(leak_pin);
        if (v != last) transitions++;
        last = v;
        delay(5);
    }

    int state = digitalRead(leak_pin);

    if (transitions > max_transitions) {
        data.leak.sensorPresent = false;
        Serial.println("[LEAK][BOOT] ❌ Signal instable -> capteur absent / câble coupé ?");
    } else {
        data.leak.sensorPresent = true;

        if (state == HIGH) {
            data.leak.leakNow = true;
            data.leak.leakLatched = true; // fuite au boot -> on verrouille
            Serial.println("[LEAK][BOOT] ⚠ FUITE detectee au demarrage !");
        } else {
            data.leak.leakNow = false;
            data.leak.leakLatched = false;
            Serial.println("[LEAK][BOOT] ✅ Capteur OK, etat sec");
        }
    }
}

// =====================
//   Initialisation
// =====================

bool Capteurs::begin()
{
    Wire.begin();

    // ===== Leak sensor (SOS BlueRobotics) =====
    pinMode(leak_pin, INPUT_PULLDOWN);
    leakBootCheck(500, 5);

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

    // ===== INA Batterie =====
    if (ina_batt.begin()) {
        ina_batt_ok = true;
        coulomb_batt.reset(100.0f);
        Serial.print("[OK] INA Batterie détecté à 0x");
        Serial.println(ina_batt_addr, HEX);
    } else {
        ina_batt_ok = false;
        Serial.print("[ERREUR] INA Batterie non détecté à 0x");
        Serial.println(ina_batt_addr, HEX);
    }

    // ===== INA Mesure =====
    if (ina_mesure.begin()) {
        ina_mesure_ok = true;
        Serial.print("[OK] INA Mesure détecté à 0x");
        Serial.println(ina_mesure_addr, HEX);
    } else {
        ina_mesure_ok = false;
        Serial.print("[ERREUR] INA Mesure non détecté à 0x");
        Serial.println(ina_mesure_addr, HEX);
    }

    // ===== MS5837 =====
    if (baro.init()) {
        depth_ok = true;
        baro.setModel(MS5837::MS5837_02BA);
        baro.setFluidDensity(997);
        Serial.println("[OK] Capteur profondeur MS5837 détecté");
    } else {
        depth_ok = false;
        Serial.println("[ERREUR] MS5837 non détecté (ignoré)");
    }

    return true;
}

// =====================
//   Calibration IMU
// =====================

void Capteurs::calibrate(bool verbose)
{
    if (!imu_ok) return;

    uint8_t sys = 0, gyro = 0, accel = 0, mag = 0;
    bno.getCalibration(&sys, &gyro, &accel, &mag);

    data.imu.sysCal   = sys;
    data.imu.gyroCal  = gyro;
    data.imu.accelCal = accel;
    data.imu.magCal   = mag;

    if (verbose) {
        Serial.print("Calibration [sys,g,a,m] = [");
        Serial.print(sys);   Serial.print(", ");
        Serial.print(gyro);  Serial.print(", ");
        Serial.print(accel); Serial.print(", ");
        Serial.print(mag);   Serial.println("]");
    }
}

// =====================
//   Update capteurs
// =====================

void Capteurs::update()
{
    // ===== Leak sensor (SOS) =====
    {
        bool leakNow = (digitalRead(leak_pin) == HIGH);
        data.leak.leakNow = leakNow;

        if (leak_latch) {
            if (leakNow) data.leak.leakLatched = true;
        } else {
            data.leak.leakLatched = leakNow;
        }
    }

    // ===== IMU =====
    if (imu_ok) {
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

        bno.getCalibration(&data.imu.sysCal, &data.imu.gyroCal, &data.imu.accelCal, &data.imu.magCal);
    }

    // ===== INA Batterie =====
    if (ina_batt_ok) {
        data.power.busVoltage_V    = ina_batt.getBusVoltage();
        data.power.shuntVoltage_mV = ina_batt.getShuntVoltage();
        data.power.current_mA      = ina_batt.getCurrent();
        data.power.power_mW        = ina_batt.getPower();

        // Si tu constates que le courant est NEGATIF en décharge, inverse ici :
        // coulomb_batt.update(-data.power.current_mA);
        coulomb_batt.update(data.power.current_mA);

        data.power.soc1_percent = coulomb_batt.get_soc();
    }

    // ===== INA Mesure =====
    if (ina_mesure_ok) {
        data.power.busVoltage2_V    = ina_mesure.getBusVoltage();
        data.power.shuntVoltage2_mV = ina_mesure.getShuntVoltage();
        data.power.current2_mA      = ina_mesure.getCurrent();
        data.power.power2_mW        = ina_mesure.getPower();
    }

    // ===== Profondeur =====
    if (depth_ok) {
        baro.read();
        data.depth.pressure_mbar = baro.pressure();
        data.depth.temperature_C = baro.temperature();
        data.depth.depth_m       = baro.depth();
    }
}

float Capteurs::getBatteryPercent() const
{
    // SoC estimé via coulomb counter (si INA batt absent => 0)
    if (!ina_batt_ok) return 0.0f;
    return data.power.soc1_percent;
}

// =====================
//   Debug
// =====================

void Capteurs::printDebug()
{
    Serial.println("====================================");

    // Leak
    Serial.print("LEAK (D"); Serial.print(leak_pin); Serial.print(") ");
    Serial.print("present="); Serial.print(data.leak.sensorPresent ? "YES" : "NO");
    Serial.print(" now="); Serial.print(data.leak.leakNow ? "LEAK" : "DRY");
    Serial.print(" latched="); Serial.println(data.leak.leakLatched ? "LEAK" : "DRY");

    // Batterie
    if (ina_batt_ok) {
        Serial.print("BAT (0x"); Serial.print(ina_batt_addr, HEX); Serial.println(")");
        Serial.print("  V="); Serial.print(data.power.busVoltage_V);
        Serial.print("V I="); Serial.print(data.power.current_mA);
        Serial.print("mA SoC="); Serial.print(data.power.soc1_percent);
        Serial.println("%");
    } else {
        Serial.println("BAT: capteur absent");
    }

    // Mesure
    if (ina_mesure_ok) {
        Serial.print("MESURE (0x"); Serial.print(ina_mesure_addr, HEX); Serial.println(")");
        Serial.print("  V="); Serial.print(data.power.busVoltage2_V);
        Serial.print("V I="); Serial.print(data.power.current2_mA);
        Serial.println("mA");
    } else {
        Serial.println("MESURE: capteur absent");
    }

    Serial.println("====================================");
}
