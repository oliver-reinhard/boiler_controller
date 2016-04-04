#ifndef BOILER_CONTROL_H_INCLUDED
  #define BOILER_CONTROL_H_INCLUDED
  
  #include "config.h"
  #include "log.h"

  typedef enum {
    CMD_NONE = 0,
    CMD_SET_CONFIG = 0x1, 
    CMD_REC_ON = 0x2,
    CMD_REC_OFF = 0x4,
    CMD_GET_LOG = 0x8, 
    CMD_GET_CONFIG = 0x10,
    CMD_GET_STAT = 0x20,
    CMD_HEAT_ON = 0x40,
    CMD_HEAT_OFF = 0x80,
    CMD_RESET = 0x100
  } UserCommandEnum;

  // OR combination ("|") of UserCommandEnum(s):
  typedef unsigned short UserCommands;

  typedef enum {
    SENSOR_INITIALISING = 0,
    SENSOR_OK = 1,
    SENSOR_NOK = 2
  } SensorStatusEnum;

  struct TemperatureSensor {
    SensorStatusEnum sensorStatus = SENSOR_INITIALISING;
    Temperature currentTemp = UNDEFINED_TEMPERATURE;
    Temperature lastLoggedTemp = UNDEFINED_TEMPERATURE;
    Timestamp lastLoggedTime = UNDEFINED_TIMESTAMP;
  };
  
  struct OperationalParams {
    TemperatureSensor water;
    TemperatureSensor ambient;
    UserCommands userCommands = CMD_NONE;
    boolean heating = false;
    boolean loggingValues = false;
  };


  /*
   * This class is stateless; all effects of actions are either immediate (to port of the Arduino chip, including serial com port), or 
   * they change operational parameter values.
   * Note: it is implemented as a class instead of normal functions for unit-testing purposes (mocking).
   */
  class ControlActions {
    public:
      virtual void readSensors(OperationalParams *op);
      virtual void readUserCommands(OperationalParams *op);
  
      virtual void heat(boolean on, OperationalParams *op);
  
      virtual void logValues(boolean on, OperationalParams *op);
  
      virtual void setConfigParam();
      virtual void getLog();
      virtual void getConfig();
      virtual void getStat();
  };
  
#endif
