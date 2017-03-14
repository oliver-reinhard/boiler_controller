#include "log.h"

//#define DEBUG_LOG

Timestamp Log::logMessage(MessageID id, int16_t param1, int16_t param2) {
  #ifdef DEBUG_LOG
    Serial.println(F("DEBUG_LOG: logMessage(..)"));
  #endif
  if (sizeof(LogMessageData) != sizeof(LogData)) {
    //Serial.print("sizeof(LogMessageData)=");
    //Serial.println(sizeof(LogMessageData));
    //Serial.print("sizeof(LogData)=");
    //Serial.println(sizeof(LogData));
    // don't use logMessage() here -> recursion!
    write_S_O_S(F("Incoherent LogMessageData size"), __LINE__); // log_S_O_S(MSG_LOG_DATA_SIZE, LOG_DATA_TYPE_MESSAGE, 0);
  }
  LogMessageData data;
  data.id = id;
  data.params[0] = param1;
  data.params[1] = param2;
  const LogEntry e = addLogEntry(LOG_DATA_TYPE_MESSAGE, (LogData *) &data);
  return e.timestamp;
}

Timestamp Log::logValues(CF_Temperature water, CF_Temperature ambient, Flags flags) {
  #ifdef DEBUG_LOG
    Serial.println(F("DEBUG_LOG: logValues(..)"));
  #endif
  if (sizeof(LogValuesData) != sizeof(LogData)) {
    log_S_O_S(MSG_LOG_DATA_SIZE, LOG_DATA_TYPE_VALUES, 0);
  }
  LogValuesData data;
  data.water = water;
  data.ambient = ambient;
  data.flags = flags;
  const LogEntry e = addLogEntry(LOG_DATA_TYPE_VALUES, (LogData *) &data);
  return e.timestamp;
}

Timestamp Log::logState(StateID previous, StateID current, EventID event) {
  #ifdef DEBUG_LOG
    Serial.println(F("DEBUG_LOG: logState(..)"));
  #endif
  if (sizeof(LogStateData) != sizeof(LogData)) {
    log_S_O_S(MSG_LOG_DATA_SIZE, LOG_DATA_TYPE_STATE, 0);
  }
  LogStateData data;
  data.previous = previous;
  data.current = current;
  data.event = event;
  const LogEntry e = addLogEntry(LOG_DATA_TYPE_STATE, (LogData *) &data);
  return e.timestamp;
}

Timestamp Log::logConfigParam(ConfigParamID id, float newValue) {
  #ifdef DEBUG_LOG
    Serial.println(F("DEBUG_LOG: logConfigParam(..)"));
  #endif
  if (sizeof(LogValuesData) != sizeof(LogData)) {
    log_S_O_S(MSG_LOG_DATA_SIZE, LOG_DATA_TYPE_CONFIG, 0);
  }
  LogConfigParamData data;
  data.id = id;
  data.newValue = newValue;
  const LogEntry e = addLogEntry(LOG_DATA_TYPE_CONFIG, (LogData *) &data);
  return e.timestamp;
}

