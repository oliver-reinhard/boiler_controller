#ifndef BOILER_CONFIG_H_INCLUDED
  #define BOILER_CONFIG_H_INCLUDED
  
  #include "Arduino.h"
  
  typedef byte ConfigParamID;
  
  typedef enum {
    PARAM_TARGET_TEMP = 0,
    PARAM_WATER_SENSOR_CUT_OUT_TEMP = 1,
    PARAM_WATER_SENSOR_BACK_OK_TEMP = 2,
    PARAM_LOG_TEMP_DELTA = 3,
    PARAM_TANK_CAPACITY = 4,
    PARAM_HEATER_POWER = 5,
    PARAM_INSULATION_FACTOR = 6
  } ConfigParam;
  
  typedef short Temperature;  // [°C / 100]
  #define UNDEFINED_TEMPERATURE -10000 // [°C / 100];
  
  typedef byte Flags;
  
  #define DEFAULT_TARGET_TEMP 4200 // [°C / 100]
  #define DEFAULT_WATER_SENSOR_CUT_OUT_TEMP 7000 // [°C / 100]
  #define DEFAULT_WATER_SENSOR_BACK_OK_TEMP 6000 // [°C / 100]
  #define DEFAULT_LOG_TEMP_DELTA 50 // [°C / 100]
  #define DEFAULT_TANK_CAPACITY 10.0 // [W]
  #define DEFAULT_HEATER_POWER 210 // [W]
  #define DEFAULT_INSULATION_FACTOR 2.0 // [???]
  
  struct ConfigParams {
    Temperature targetTemp;
    Temperature waterSensorCutOutTemp;
    Temperature waterSensorBackOkTemp;
    Temperature logTempDelta;
    float tankCapacity;  // Litres
    float heaterPower;  // Watts
    float insulationFactor; // XXX factor
    byte  reserved[32];
  };

#endif
