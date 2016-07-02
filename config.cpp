#include "config.h"


void ConfigParams::setTempSensorIDs(TempSensorID water, TempSensorID ambient) {
  memcpy(this->waterTempSensorId, water, TEMP_SENSOR_ID_BYTES);
  memcpy(this->ambientTempSensorId, ambient, TEMP_SENSOR_ID_BYTES);
}

void ConfigParams::initParams(boolean &updated) {
  updated = false;
  
  if (targetTemp == 0) {
    targetTemp = DEFAULT_TARGET_TEMP;
    updated = true;
  }

  // Temp Sensor IDs do not have a default value => need to be set as part of the installation.

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
  AbstractConfigParams::print();
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

