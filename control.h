#ifndef BOILER_CONTROL_H_INCLUDED
  #define BOILER_CONTROL_H_INCLUDED
  
  #include "bc_setup.h"
  #include <OneWire.h>
  #include "config.h"
  #include "store.h"

  #define HEATER_PIN   9
  #define ONE_WIRE_PIN 10 // Temperature sensors
  
  #define CMD_ARG_BUF_SIZE 30   // Size of the read buffer for incoming data
  
  typedef enum {
    CMD_NONE = 0,          // 1
    CMD_HELP = 0x1,        // 2
    CMD_SET_CONFIG = 0x2,  // 3
    CMD_REC_ON = 0x4,      // 4
    CMD_REC_OFF = 0x8,     // 5
    CMD_GET_LOG = 0x10,    // 6  (16)
    CMD_GET_CONFIG = 0x20, // 7  (32)
    CMD_GET_STAT = 0x40,   // 8  (64)
    CMD_HEAT_ON = 0x80,    // 9  (128)
    CMD_HEAT_OFF = 0x100,  // 10 (256)
    CMD_RESET = 0x200,     // 11 (512)
  } UserCommandEnum;
  
  const uint16_t NUM_USER_COMMANDS = 11;

  // bitwise OR combination ("|") of UserCommandEnum(s):
  typedef uint16_t UserCommands;

  typedef enum {
    READ_WRITE_NONE = 0,
    READ_HELP = 0x1,
    READ_STAT = 0x2,
    READ_LOG = 0x4,
    READ_CONFIG = 0x8,
    WRITE_CONFIG = 0x10
  } ReadWriteRequestEnum;

  // bitwise OR combination ("|") of ReadWriteRequestEnum(s)
  typedef uint8_t ReadWriteRequests;

  typedef enum {
    SENSOR_INITIALISING = 0,
    SENSOR_OK = 1,
    SENSOR_NOK = 2,
    SENSOR_ID_UNDEFINED = 3
  } SensorStatusEnum;

  typedef uint8_t SensorStatusID;
  
  struct TemperatureSensor {
    SensorStatusEnum sensorStatus = SENSOR_INITIALISING;
    Temperature currentTemp = UNDEFINED_TEMPERATURE;
    Temperature lastLoggedTemp = UNDEFINED_TEMPERATURE;
    uint32_t lastLoggedTime = 0L;
  };

  struct UserCommand {
    UserCommandEnum command = CMD_NONE;
    char args[CMD_ARG_BUF_SIZE]; // always '\0' terminated
  };
  
  struct OperationalParams {
    // timepoint [ms] of most recent transition to current state:
    uint32_t currentStateStartMillis = 0L;
    TemperatureSensor water;
    TemperatureSensor ambient;
    UserCommand *command;
    boolean heating = false;
    // time [ms] of most recent transition to state HEATING:
    uint32_t heatingStartMillis = 0L;
    // accumulated time in state HEATING, not including the period since last start (if heatingStartMillis != 0L)
    uint32_t heatingAccumulatedMillis = 0L;
    boolean loggingValues = false;
  };

  /*
   * Calculates current heating time.
   */
  uint32_t heatingTotalMillis(OperationalParams *op);

  class ControlContext {
    public:
      Storage *storage;
      ConfigParams *config;
      OperationalParams *op;
  };
  
  struct TemperatureReadout {
    uint8_t resolution;     // number of bits (= 9..12)
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
  
      virtual void setConfigParam();
      virtual void requestHelp();
      virtual void requestLog();
      virtual void requestConfig();
      virtual void requestStat();
      ReadWriteRequests getPendingReadWriteRequests();
      void clearPendingReadWriteRequests();
      
    protected:
      OneWire oneWire = OneWire(ONE_WIRE_PIN);  // on pin 10 (a 4.7K pull-up resistor to 5V is necessary)
      boolean readScratchpad(uint8_t addr[], uint8_t *data);
      TemperatureReadout getCelcius(uint8_t data[]);
      ReadWriteRequests pendingRequests = READ_WRITE_NONE;
  };
  
#endif
