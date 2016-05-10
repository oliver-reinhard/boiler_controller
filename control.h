#ifndef BOILER_CONTROL_H_INCLUDED
  #define BOILER_CONTROL_H_INCLUDED
  
  #include <OneWire.h>
  #include "config.h"
  #include "storage.h"

  #define HEATER_PIN 9
  #define ONE_WIRE_PIN 10 // Temperature sensors
  
  #define CMD_ARG_BUF_SIZE 30   // Size of the read buffer for incoming data
  
  typedef enum {
    CMD_NONE = 0,
    CMD_HELP = 0x1,
    CMD_SET_CONFIG = 0x2, 
    CMD_REC_ON = 0x4,
    CMD_REC_OFF = 0x8,
    CMD_GET_LOG = 0x10, 
    CMD_GET_CONFIG = 0x20,
    CMD_GET_STAT = 0x40,
    CMD_HEAT_ON = 0x80,
    CMD_HEAT_OFF = 0x100,
    CMD_RESET = 0x200,
  } UserCommandEnum;
  
  const unsigned short NUM_USER_COMMANDS = 10;

  // bitwise OR combination ("|") of UserCommandEnum(s):
  typedef unsigned short UserCommands;

  typedef enum {
    READ_WRITE_NONE = 0,
    READ_HELP = 0x1,
    READ_STAT = 0x2,
    READ_LOG = 0x4,
    READ_CONFIG = 0x8,
    WRITE_CONFIG = 0x10
  } ReadWriteRequestEnum;

  // bitwise OR combination ("|") of ReadWriteRequestEnum(s)
  typedef byte ReadWriteRequests;

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

  struct UserCommand {
    UserCommandEnum command = CMD_NONE;
    char args[CMD_ARG_BUF_SIZE]; // always '\0' terminated
  };
  
  struct OperationalParams {
    // time [ms] of most recent transition to current state:
    unsigned long currentStateStartMillis = 0L;
    TemperatureSensor water;
    TemperatureSensor ambient;
    UserCommand *command;
    boolean heating = false;
    // time [ms] of most recent transition to state HEATING:
    unsigned long heatingStartMillis = 0L;
    // accumulated time in state HEATING except period since last start (if heatingStartMillis != 0L)
    unsigned long heatingTotalMillis = 0L;
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
      virtual void requestHelp();
      virtual void requestLog();
      virtual void requestConfig();
      virtual void requestStat();
      ReadWriteRequests getPendingReadWriteRequests();
      void clearPendingReadWriteRequests();
      
    protected:
      OneWire oneWire = OneWire(ONE_WIRE_PIN);  // on pin 10 (a 4.7K pull-up resistor to 5V is necessary)
      boolean readScratchpad(byte addr[], byte *data);
      TemperatureReadout getCelcius(byte data[]);
      ReadWriteRequests pendingRequests = READ_WRITE_NONE;
  };
  
#endif
