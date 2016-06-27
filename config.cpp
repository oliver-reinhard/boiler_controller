#include "config.h"

#define ASCII_0 48  // char(48)


void ConfigParams::initParams(boolean &updated) {
  updated = false;
  
  if (targetTemp == 0) {
    targetTemp = DEFAULT_TARGET_TEMP;
    updated = true;
  }

  // Temp Sensor IDs do not have a default value => need to be set as part of the installation .

  if (heaterCutOutWaterTemp == 0) {
    heaterCutOutWaterTemp = DEFAULT_HEATER_CUT_OUT_WATER_TEMP;
    updated = true;
  }

  if (heaterBackOkWaterTemp == 0) {
    heaterBackOkWaterTemp = DEFAULT_HEATER_BACK_OK_WATER_TEMP;
    updated = true;
  }

  if (logTempDelta == 0) {
    logTempDelta = DEFAULT_LOG_TEMP_DELTA;
    updated = true;
  }

  if (logTimeDelta == 0L) {
    logTimeDelta = DEFAULT_LOG_TIME_DELTA;
    updated = true;
  }
  
  if (tankCapacity == 0.0) {
    tankCapacity = DEFAULT_TANK_CAPACITY;
    updated = true;
  }

  if (heaterPower == 0.0) {
    heaterPower = DEFAULT_HEATER_POWER;
    updated = true;
  }

  if (insulationFactor == 0.0) {
    insulationFactor = DEFAULT_INSULATION_FACTOR;
    updated = true;
  }

  // *** Initialise new config parameters here:
}


void ConfigParams::print() {
  AbstractConfiguration::print();
  Serial.println(formatTemperature(targetTemp));
  Serial.println(formatTempSensorID(waterTempSensorId));
  Serial.println(formatTempSensorID(ambientTempSensorId));
  Serial.println(formatTemperature(heaterCutOutWaterTemp));
  Serial.println(formatTemperature(heaterBackOkWaterTemp));
  Serial.println(formatTemperature(logTempDelta));
  Serial.println(logTimeDelta);
  Serial.println(tankCapacity);
  Serial.println(heaterPower);
  Serial.println(insulationFactor);
}


String formatTemperature(Temperature t) {
  char s[9];
  uint8_t deg = t / 100;
  uint8_t frac = t % 100;
  s[8] = '\0';
  s[7] = 'C';
  s[6] = '\'';
  s[5] = ASCII_0 + frac % 10;
  s[4] = ASCII_0 + frac / 10;
  s[3] = '.';
  s[2] = ASCII_0 + deg % 10;
  s[1] = deg >= 10 ? ASCII_0 + deg / 10 : ' ';
  s[0] = t > 0 ? ' ' : '-';
  return s;
}


String formatTempSensorID(TempSensorID id) {
  char s[3*TEMP_SENSOR_ID_BYTES];
  uint8_t pos = 0;
  for (uint8_t i=0; i<TEMP_SENSOR_ID_BYTES; i++) {
    sprintf(&s[pos], "%02X", id[i]);
    pos += 2;
    if (i < TEMP_SENSOR_ID_BYTES - 1) {
      s[pos++] = '-';
    }
  }
  s[pos++] = '\0';
  return s;
}
