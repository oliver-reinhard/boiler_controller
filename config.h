#ifndef BOILER_CONFIG_H_INCLUDED
  #define BOILER_CONFIG_H_INCLUDED
  
  #include "Arduino.h"
  
  typedef short Temperature;  // [°C * 100]
  #define UNDEFINED_TEMPERATURE -10000 // [°C * 100];

  #define TEMP_SENSOR_ID_BYTES 8
  typedef byte TempSensorID[TEMP_SENSOR_ID_BYTES];
  const TempSensorID UNDEFINED_SENSOR_ID = {0,0,0,0,0,0,0,0};

  // Water min and max values used to check that sensor-temperature readout is plausible:
  #define WATER_MIN_TEMP -2000 // [°C * 100]
  #define WATER_MAX_TEMP 10000 // [°C * 100]
  
  // Water min and max values used to check that ambient-temperature readout is plausible:
  #define AMBIENT_MIN_TEMP -3000 // [°C * 100]
  #define AMBIENT_MAX_TEMP 5000 // [°C * 100]
  
  /*
   * IDs are defined by ConfigParamEnum.
   */
  typedef byte ConfigParamID;
  
  typedef enum {
    PARAM_NONE = -1,
    PARAM_TARGET_TEMP = 0,
    PARAM_WATER_TEMP_SENSOR_ID = 1,
    PARAM_AMBIENT_TEMP_SENSOR_ID = 2,
    PARAM_HEATER_CUT_OUT_WATER_TEMP = 3,
    PARAM_HEATER_BACK_OK_WATER_TEMP = 4,
    PARAM_LOG_TEMP_DELTA = 5,
    PARAM_LOG_TIME_DELTA = 6,
    PARAM_TANK_CAPACITY = 7,
    PARAM_HEATER_POWER = 8,
    PARAM_INSULATION_FACTOR = 9
  } ConfigParamEnum;
  
  const byte NUM_CONFIG_PARAMS = 10;
  
  struct ConfigParams {
    Temperature targetTemp; // [°C * 100]
    TempSensorID waterTempSensorId;
    TempSensorID ambientTempSensorId;
    Temperature heaterCutOutWaterTemp; // [°C * 100]
    Temperature heaterBackOkWaterTemp; // [°C * 100]
    Temperature logTempDelta; // [°C * 100]
    unsigned short logTimeDelta; // [s]
    float tankCapacity;  // [litre]
    float heaterPower;  // [Watts]
    float insulationFactor; // correction factor to model tank insulation charactericstis
    byte  reserved[32];  // for future use
  };
  
  #define DEFAULT_TARGET_TEMP                 4200 // [°C * 100]
  #define DEFAULT_WATER_TEMP_SENSOR_ID        UNDEFINED_SENSOR_ID
  #define DEFAULT_AMBIENT_TEMP_SENSOR_ID      UNDEFINED_SENSOR_ID
  #define DEFAULT_HEATER_CUT_OUT_WATER_TEMP   7000 // [°C * 100]
  #define DEFAULT_HEATER_BACK_OK_WATER_TEMP   6000 // [°C * 100]
  #define DEFAULT_LOG_TEMP_DELTA              50 // [°C * 100]
  #define DEFAULT_LOG_TIME_DELTA              60 // [s]
  #define DEFAULT_TANK_CAPACITY               10.0 // [litre]
  #define DEFAULT_HEATER_POWER                210 // [W]
  #define DEFAULT_INSULATION_FACTOR           2.0 // [???]


  /**
   * Returns the temperature in a dotted notation, 8 chars + terminal '\0': ["-"]dd.ff"°C" (optional "-", d = degrees Celsius (2 digits), f = fraction (1 digit))
   */
  String formatTemperature(Temperature t);

  String formatTempSensorID(TempSensorID id);
  
  void printConfig(ConfigParams p);
  
#endif
