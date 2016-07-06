#ifndef BC_CONTROL_H_INCLUDED
  #define BC_CONTROL_H_INCLUDED
  
  #include "bc_setup.h"
  #include "config.h"
  #include "log.h"

  #define HEATER_PIN   9
  #define ONE_WIRE_PIN 10 // Temperature sensors

  typedef enum {
    MSG_WATER_TEMP_SENSOR_SILENT = 20,      // Water-temperature sensor not responding. Check wiring and sensor, then restart.
    MSG_WATER_TEMP_SENSOR_ID_UNDEF = 21,    // Sensor ID of water-temperature sensor could not be obtained. Check wiring and sensor, then restart.
    MSG_WATER_TEMP_SENSOR_ID_AUTO =  22,    // Sensor ID of water-temperature sensor was assigned automatically. Confirm that it refers to the correct sensor, then restart.
    MSG_AMBIENT_TEMP_SENSOR_SILENT = 23,    // Ambient-temperature sensor not responding. Check wiring and sensor, then restart.
    MSG_AMBIENT_TEMP_SENSOR_ID_UNDEF = 24,  // Sensor ID of ambient-temperature sensor could not be obtained. Check wiring and sensor, then restart.
    MSG_AMBIENT_TEMP_SENSOR_ID_AUTO =  25,  // Sensor ID of ambient-temperature sensor was assigned automatically. Confirm that it refers to the correct sensor, then restart.
    MSG_TEMP_SENSOR_IDS_CLEARED =  26       // Sensor IDs of water- and ambient-temperature sensor cleared. Restart to see the effect.
  } ControlMessageEnum;


  // Water min and max values used to check that sensor-temperature readout is plausible:
  #define WATER_MIN_TEMP -2000 // [°C * 100]
  #define WATER_MAX_TEMP 10000 // [°C * 100]
  
  // Water min and max values used to check that ambient-temperature readout is plausible:
  #define AMBIENT_MIN_TEMP -3000 // [°C * 100]
  #define AMBIENT_MAX_TEMP 5000 // [°C * 100]
  
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

  struct UserCommand {
    UserCommandEnum command = CMD_NONE;
    char args[CMD_ARG_BUF_SIZE]; // always '\0' terminated
  };
  
  struct OperationalParams {
    // timepoint [ms] of most recent transition to current state:
    uint32_t currentStateStartMillis = 0L;
    DS18B20TemperatureSensor water = DS18B20TemperatureSensor("Water");
    DS18B20TemperatureSensor ambient = DS18B20TemperatureSensor("Ambient");
    UserCommand *command;
    boolean heating = false;
    // time [ms] of most recent transition to state HEATING:
    uint32_t heatingStartMillis = 0L;
    // accumulated time in state HEATING, not including the period since last start (if heatingStartMillis != 0L)
    uint32_t heatingAccumulatedMillis = 0L;
    boolean loggingValues = false;

    /*
     * Swap sensor IDs and sensor states between water and ambient sensor.
     */
    void swapTempSensorIDs();
  };

  /*
   * Calculates current heating time.
   */
  uint32_t heatingTotalMillis(OperationalParams *op);

  class ControlContext {
    public:
      Log *log;
      ConfigParams *config;
      OperationalParams *op;
      DS18B20Controller *controller;
  };

  /*
   * This class is stateless; all effects of actions are either immediate (to a port of the Arduino chip, including serial com port), or 
   * they change operational parameter values.
   * Note: it is implemented as a class instead of normal functions for unit-testing purposes (mocking).
   */
  class ControlActions {
    public:
      ControlActions(ControlContext *context) {
        this->context = context;
      }

      /*
       * Performs a search on the OneWire Bus and matches / assigns returned sensor addresses with configured sensors.
       */
      uint8_t setupSensors();

      /*
       * Swap the sensor IDs and states of water- and ambient-temperature sensor in the operational parameters.
       */
      void swapTempSensorIDs();

      /*
       * Clears the sensor IDs of water- and ambient-temperature sensors in the configuration parameters and saves the latter to EEPROM. 
       */
      void clearTempSensorIDs();
      
      /*
       * Copies the sensor IDs of water- and ambient-temperature sensors of the the configuration parameters to the configuration parameters and saves the latter to EEPROM. 
       * Sets the sensor status of one or both sensors to SENSOR_OK iff sensor status is SENSOR_ID_AUTO_ASSIGNED.
       * 
       * @return true if at lestt one ID was copied and status was changed.
       */
      boolean confirmTempSensorIDs();

      /*
       * Sends a command to all physical temperature sensors to trigger the conversion of physical values to a temperature.
       * 
       * Note: The conversion takes time (up to 750 ms depending on the precision of the returned value) and completeSensorReadout cannot be invoked before that.
       *       See the data sheet of the DS18B20 temperature sensors for exact times.
       */
      void initSensorReadout();

      /*
       * Reads the temperatures from the physical sensors and assignes the value and state to the configured temperature sonsors.
       * 
       * Note: See the note for initSensorReadout()
       */
      void completeSensorReadout();

      /*
       * Physically turns the water heater on or off.
       */
      virtual void heat(boolean on);
  
      virtual void setConfigParam();
      virtual void requestHelp();
      virtual void requestLog();
      virtual void requestConfig();
      virtual void requestStat();
      ReadWriteRequests getPendingReadWriteRequests();
      void clearPendingReadWriteRequests();
      
    protected:
      ControlContext *context;
      ReadWriteRequests pendingRequests = READ_WRITE_NONE;

      void logSensorSetupIssue(DS18B20TemperatureSensor *sensor, ControlMessageEnum msg);
  };
  
#endif
