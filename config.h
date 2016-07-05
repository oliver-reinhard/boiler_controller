#ifndef BC_CONFIG_H_INCLUDED
  #define BC_CONFIG_H_INCLUDED
  
  #include "config/Configuration.h"
  #include "sensors/OneWireSensors.h"
  
  /*
   * ID values are defined by ConfigParamEnum.
   */
  typedef uint8_t ConfigParamID;
  
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
  } ConfigParamEnum;
  
  const uint8_t NUM_CONFIG_PARAMS = 9;
  
  #define DEFAULT_TARGET_TEMP                 4200 // [°C * 100]
  #define DEFAULT_WATER_TEMP_SENSOR_ID        UNDEFINED_SENSOR_ID
  #define DEFAULT_AMBIENT_TEMP_SENSOR_ID      UNDEFINED_SENSOR_ID
  #define DEFAULT_HEATER_CUT_OUT_WATER_TEMP   7000 // [°C * 100]
  #define DEFAULT_HEATER_BACK_OK_WATER_TEMP   6000 // [°C * 100]
  #define DEFAULT_LOG_TEMP_DELTA              50 // [°C * 100]
  #define DEFAULT_LOG_TIME_DELTA              60 // [s]
  #define DEFAULT_TANK_CAPACITY               10.0 // [litre]
  #define DEFAULT_HEATER_POWER                210 // [W]

  
  class ConfigParams : public AbstractConfigParams {
    public:
      ConfigParams() : AbstractConfigParams(0L) { };
      
      Temperature targetTemp; // [°C * 100]
      TempSensorID waterTempSensorId;
      TempSensorID ambientTempSensorId;
      Temperature heaterCutOutWaterTemp; // [°C * 100]
      Temperature heaterBackOkWaterTemp; // [°C * 100]
      Temperature logTempDelta; // [°C * 100]
      uint16_t logTimeDelta; // [s]
      float tankCapacity;  // [litre]
      float heaterPower;  // [Watts]
      uint8_t  reserved[32];  // for future use

      void setTempSensorIDs(TempSensorID water, TempSensorID ambient);
      
      /* Override. */
      void print();
      
      /* Override. */
      uint16_t memSize() { return sizeof(*this); };
      
    protected:
      void initParams(boolean &updated);
  };
  
#endif
