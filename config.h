#ifndef BOILER_CONFIG_H_INCLUDED
  #define BOILER_CONFIG_H_INCLUDED
  
  #include "Arduino.h"
  
  typedef byte ConfigParamID;
  
  typedef enum {
    PARAM_TARGET_TEMP = 0,
    PARAM_WATER_TEMP_SENSOR_ID = 1,
    PARAM_AMBIENT_TEMP_SENSOR_ID = 2,
    PARAM_HEATER_CUT_OUT_WATER_TEMP = 3,
    PARAM_HEATER_BACK_OK_WATER_TEMP = 4,
    PARAM_LOG_TEMP_DELTA = 5,
    PARAM_TANK_CAPACITY = 6,
    PARAM_HEATER_POWER = 7,
    PARAM_INSULATION_FACTOR = 8
  } ConfigParam;
  
  typedef short Temperature;  // [°C / 100]
  #define UNDEFINED_TEMPERATURE -10000 // [°C / 100];

  #define TEMP_SENSOR_ID_BYTES 8
  typedef byte TempSensorID[TEMP_SENSOR_ID_BYTES];
  const TempSensorID UNDEFINED_SENSOR_ID = {0,0,0,0,0,0,0,0};
  
  typedef byte Flags;
  
  #define DEFAULT_TARGET_TEMP 4200 // [°C / 100]
  #define DEFAULT_WATER_TEMP_SENSOR_ID {1,1,1,1,1,1,1,1}
  #define DEFAULT_AMBIENT_TEMP_SENSOR_ID {2,2,2,2,2,2,2,2}
  #define DEFAULT_HEATER_CUT_OUT_WATER_TEMP 7000 // [°C / 100]
  #define DEFAULT_HEATER_BACK_OK_WATER_TEMP 6000 // [°C / 100]
  #define DEFAULT_LOG_TEMP_DELTA 50 // [°C / 100]
  #define DEFAULT_TANK_CAPACITY 10.0 // [W]
  #define DEFAULT_HEATER_POWER 210 // [W]
  #define DEFAULT_INSULATION_FACTOR 2.0 // [???]
  
  struct ConfigParams {
    Temperature targetTemp;
    TempSensorID waterTempSensor;
    TempSensorID ambientTempSensor;
    Temperature heaterCutOutWaterTemp;
    Temperature heaterBackOkWaterTemp;
    Temperature logTempDelta;
    float tankCapacity;  // Litres
    float heaterPower;  // Watts
    float insulationFactor; // XXX factor
    byte  reserved[32];
  };

#endif
