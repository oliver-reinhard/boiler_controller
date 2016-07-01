#ifndef BC_LOG_H_INCLUDED
  #define BC_LOG_H_INCLUDED

  #include "config.h"
  #include "log/Logging.h"

  
  /*
   * The messages issued by the implementation of this module. Stored as MessageID.
   * Note: the actual message texts and the conversion of message IDs to text have to be implemented by 
   *       the consumer of this libraray 
   */
  typedef enum {
    MSG_LOG_DATA_SIZE = 10   // Size of LogData subtype does not correspond to sizeof(LogData); subtype = [LogDataTypeEnum]
  } LogMessageEnum;
  
  #define LOG_DATA_PAYLOAD_SIZE 5
  
  typedef enum {
    LOG_DATA_TYPE_MESSAGE = 0,
    LOG_DATA_TYPE_VALUES = 1,
    LOG_DATA_TYPE_STATE = 2,
    LOG_DATA_TYPE_CONFIG = 3,
  } LogDataTypeEnum;


  /*
   * LogDataType: LOG_DATA_TYPE_MESSAGE, "subtype" of LogData
   */
  struct LogMessageData {
    MessageID id;
    int16_t params[2];
  };
  
  typedef uint8_t Flags; // value logging
  
  /*
   * LogDataType: LOG_DATA_TYPE_VALUES, "subtype" of LogData
   */
  struct LogValuesData {
    Temperature water;
    Temperature ambient;
    Flags flags;
  };
  
  /*
   * IDs for enum types, some defined in higher-level modules.
   */
  typedef uint8_t StateID;
  typedef uint16_t EventID;
  
  /**
   * LogDataType: LOG_DATA_TYPE_STATE, "subtype" of LogData
   */
  struct LogStateData {
    StateID previous;
    StateID current;
    EventID event;
    uint8_t unused; // filler uint8_t
  };
  
  /**
   * LogDataType: LOG_DATA_TYPE_CONFIG, "subtype" of LogData
   */
  struct LogConfigParamData {
    ConfigParamID id;
    float newValue;
  };

  
  class Log : public AbstractLog {
    public:
      Log(uint16_t eepromOffset) : AbstractLog(eepromOffset) { };

      /*
       * Log a message.
       */
      virtual Timestamp logMessage(MessageID msg, int16_t param1, int16_t param2);

      /*
       * Log a value change.
       */
      virtual Timestamp logValues(Temperature water, Temperature ambient, Flags flags);

      /*
       * Log a state change.
       */
      virtual Timestamp logState(StateID previous, StateID current, EventID event);

      /*
       * Log a config-param change.
       */
      virtual Timestamp logConfigParam(ConfigParamID id, float newValue);     
  };

#endif
