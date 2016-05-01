#ifndef BOILER_CONTROL_H_INCLUDED
  #define BOILER_CONTROL_H_INCLUDED
  
  #include <OneWire.h>
  #include "config.h"
  #include "storage.h"

  #define HEATER_PIN 9
  #define ONE_WIRE_PIN 10 // Temperature sensors

  // Water min and max values used to check that sensor-temperature readout is plausible:
  #define WATER_MIN_TEMP -2000 // [°C * 100]
  #define WATER_MAX_TEMP 10000 // [°C * 100]
  
  // Water min and max values used to check that ambient-temperature readout is plausible:
  #define AMBIENT_MIN_TEMP -3000 // [°C * 100]
  #define AMBIENT_MAX_TEMP 5000 // [°C * 100]
  
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

  // bitwise OR combination ("|") of UserCommandEnum(s):
  typedef unsigned short UserCommands;

  typedef enum {
    SENSOR_INITIALISING = 0,
    SENSOR_OK = 1,
    SENSOR_NOK = 2,
    SENSOR_ID_UNDEFINED = 3
  } SensorStatusEnum;

  
  struct TemperatureSensor {
    SensorStatusEnum sensorStatus = SENSOR_INITIALISING;
    Temperature currentTemp = UNDEFINED_TEMPERATURE;
    Temperature lastLoggedTemp = UNDEFINED_TEMPERATURE;
    unsigned long lastLoggedTime = 0L;
  };
  
  struct OperationalParams {
    TemperatureSensor water;
    TemperatureSensor ambient;
    UserCommands userCommands = CMD_NONE;
    boolean heating = false;
    boolean loggingValues = false;
  };

  class ControlContext {
    public:
      Storage *storage;
      ConfigParams *config;
      OperationalParams *op;
  };
  
  struct TemperatureReadout {
    byte resolution;     // number of bits (= 9..12)
    Temperature celcius; // [°C * 100]
  };

  /*
   * This class is stateless; all effects of actions are either immediate (to a port of the Arduino chip, including serial com port), or 
   * they change operational parameter values.
   * Note: it is implemented as a class instead of normal functions for unit-testing purposes (mocking).
   */
  class ControlActions {
    public:
      virtual void setupSensors(ControlContext *context);
      virtual void initSensorReadout(ControlContext *context);
      virtual void completeSensorReadout(ControlContext *context);
      virtual void readUserCommands(ControlContext *context);

      /*
       * Physically turns the water heater on or off.
       */
      virtual void heat(boolean on, ControlContext *context);

      /*
       * Checks whether
       * - logging is turned on or off
       * - values have changed sufficiently to warrant logging (context->config->logTempDelta)
       * - enough time has elapsed for a new logging record (context->config->logTimeDelta)
       */
      virtual void logTemperatureValues(ControlContext *context);
  
      virtual void setConfigParam();
      virtual void getLog();
      virtual void getConfig();
      virtual void getStat();
    protected:
      OneWire oneWire = OneWire(ONE_WIRE_PIN);  // on pin 10 (a 4.7K pull-up resistor to 5V is necessary)
      boolean readScratchpad(byte addr[], byte *data);
      TemperatureReadout getCelcius(byte data[]);
  };
  
#endif
