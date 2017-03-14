#ifndef BC_CONFIG_H_INCLUDED
  #define BC_CONFIG_H_INCLUDED
  
  #include <CF_Configuration.h>
  #include "src/sensors/DS18B20.h"
  
  /*
   * ID values are defined by ConfigParamEnum.
   */
  typedef uint8_t ConfigParamID;
  
  typedef enum {
    PARAM_NONE = -1,
    PARAM_WATER_TEMP_SENSOR_ID = 0,
    PARAM_AMBIENT_TEMP_SENSOR_ID = 1,
    PARAM_TARGET_TEMP = 2,
    PARAM_HEATER_CUT_OUT_WATER_TEMP = 3,
    PARAM_HEATER_BACK_OK_WATER_TEMP = 4,
    PARAM_LOG_TEMP_DELTA = 5,
    PARAM_LOG_TIME_DELTA = 6,
    PARAM_TANK_CAPACITY = 7,
    PARAM_HEATER_POWER = 8
  } ConfigParamEnum;
  
  const uint8_t NUM_CONFIG_PARAMS = 9;
  
  #define DEFAULT_WATER_TEMP_SENSOR_ID        DS18B20_UNDEFINED_SENSOR_ID
  #define DEFAULT_AMBIENT_TEMP_SENSOR_ID      DS18B20_UNDEFINED_SENSOR_ID
  #define DEFAULT_TARGET_TEMP                 4200 // [°C * 100]
  #define DEFAULT_HEATER_CUT_OUT_WATER_TEMP   7000 // [°C * 100]
  #define DEFAULT_HEATER_BACK_OK_WATER_TEMP   6000 // [°C * 100]
  #define DEFAULT_LOG_TEMP_DELTA              50 // [°C * 100]
  #define DEFAULT_LOG_TIME_DELTA              60 // [s]
  #define DEFAULT_TANK_CAPACITY               10.0 // [litre]
  #define DEFAULT_HEATER_POWER                204 // [W]

  typedef enum {
    PARAM_TYPE_UNDEFINED = -1,
    PARAM_TYPE_TEMP_SENSOR_ID = 0,
    PARAM_TYPE_TEMPERATURE = 1,
    PARAM_TYPE_FLOAT = 2
  } ConfigParamTypeEnum;

  
  #define CONFIG_PARAMS_VERSION 3
  
  class ConfigParams : public AbstractConfigParams {
    public:
      ConfigParams(AbstractStore *store) : AbstractConfigParams(store, CONFIG_PARAMS_VERSION) { };
      
      DS18B20_SensorID waterTempSensorId;
      DS18B20_SensorID ambientTempSensorId;
      CF_Temperature targetTemp; // [°C * 100]
      CF_Temperature heaterCutOutWaterTemp; // [°C * 100]
      CF_Temperature heaterBackOkWaterTemp; // [°C * 100]
      CF_Temperature logTempDelta; // [°C * 100]
      uint16_t logTimeDelta; // [s]
      float tankCapacity;  // [litre]
      float heaterPower;  // [Watts]
      uint8_t  reserved[32];  // for future use

      /* Returns the type of a given parameter. */
      ConfigParamTypeEnum paramType(ConfigParamEnum param);

      /* Sets the sensor IDs for both temperature sensors. */
      void setDS18B20_SensorIDs(DS18B20_SensorID water, DS18B20_SensorID ambient);

      /* Clears als parameter values (see clear()), then initialises all values (see initParams(.)). */
      void reset();
      
      /* Override. */
      void print();
      
      /* Override. */
      uint16_t memSize() { return sizeof(*this); };
      
    protected:
      void initParams(boolean &updated);
  };
  
#endif
