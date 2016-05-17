#ifndef BOILER_LOG_H_INCLUDED
  #define BOILER_LOG_H_INCLUDED
  
  #include "msg.h"
  #include "config.h"
  
  /**
   * Timestamps are unique identifiers across the full time range of this type.
   * 
   * Timestamp format: 28 bits [seconds since last log reset] + 4 bits [uniquness identifier 0..15].
   *                   - 2^28 seconds is about 8.5 years
   *                   - the log reset may have happened several board resets earlier
   *                   - up to 16 timestamps can be generated within one second
   * The timestamp() function creates ascending values in strong monotony, even across board resets.
   */
  typedef unsigned long Timestamp;
  #define TIMESTAMP_ID_BITS   4
  #define UNDEFINED_TIMESTAMP 0L
  
  struct LogTimeRaw {
    unsigned long sec; // seconds since log was last reset or since last board reset, which ever happened earlier
    unsigned short ms; // milliseconds [0 .. 999]
  };
  
  typedef enum {
    LOG_VALUES = 0,
    LOG_STATE = 1,
    LOG_MESSAGE = 2,
    LOG_CONFIG = 3,
  } LogTypeEnum;

  /*
   * IDs for enum types, some defined in higher-level modules.
   */
  typedef byte LogTypeID;
  typedef byte MessageID;
  typedef byte StateID;
  typedef unsigned short EventID;
  
  typedef byte Flags; // value logging

  
  /**
   * Generic "supertype" for log data.
   */
  struct LogData {
    char data[5]; // placeholder
  };
  
  /**
   * LogType: LOG_VALUES, "subtype" of LogData
   */
  struct LogValuesData {
    Temperature water;
    Temperature ambient;
    Flags flags;
  };
  
  /**
   * LogType: LOG_STATE, "subtype" of LogData
   */
  struct LogStateData {
    StateID previous;
    StateID current;
    EventID event;
    byte unused; // filler byte
  };
  
  /**
   * LogType: LOG_MESSAGE, "subtype" of LogData
   */
  struct LogMessageData {
    MessageID id;
    short params[2];
  };
  
  /**
   * LogType: LOG_CONFIG, "subtype" of LogData
   */
  struct LogConfigParamData {
    ConfigParamID id;
    float newValue;
  };
  
  /**
   * Actual log record. At runtime the data field is an instance of a "subtype" of LogData.
   */
  struct LogEntry {
    Timestamp timestamp;
    LogTypeID type;
    LogData data; // generic
  };
  
  /**
   * Calculates a time offset from the mostRecent log-entry timestamp (which usually stems from a 
   * log entry prior to board reset or power down).
   */
  void adjustLogTime(Timestamp mostRecent);
  
  /**
   * Resets the offset to 0 which means that log time and millis() coincide again in terms of seconds elapsed.
   */
  void resetLogTime();
  
  /**
   * Returns the raw log time in internal format.
   */
  LogTimeRaw logTime();
  
  /**
   * Returns a unique timestamp. A maximum of 16 timestamps can be generated within the same second.
   * 
   * Note: this function will delay() until the next second starts (where millis() % 1000 == 0) if the 
   * maximum of 16 timestamps in a given second is exceeded.
   */
  Timestamp timestamp();

  /**
   * Returns the timestamp in a 13-character dotted notation, terminated by '\0': sssssssss.cc  (s = seconds: 2^28 = 268435456 (9 digits), cc = count: 0..15 (2 digits))
   */
 String formatTimestamp(Timestamp t);
  
  /**
   * Returns a LogEntry with a data field of "type" LogValuesData.
   */
  LogEntry createLogValuesEntry(Temperature water, Temperature ambient, Flags flags);

  /**
   * Returns a LogEntry with a data field of "type" LogStateData.
   */
  LogEntry createLogStateEntry(StateID previous, StateID current, EventID event);

  /**
   * Returns a LogEntry with a data field of "type" LogMessageData.
   */
  LogEntry createLogMessageEntry(MessageEnum id, short param1, short param2);

  /**
   * Returns a LogEntry with a data field of "type" LogConfigParamData.
   */
  LogEntry createConfigParamEntry(ConfigParamID id, float newValue);

#endif
