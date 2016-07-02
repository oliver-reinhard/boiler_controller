#include "log.h"

//#define DEBUG_LOG

Timestamp Log::logMessage(MessageID id, int16_t param1, int16_t param2) {
  if (sizeof(LogMessageData) != sizeof(LogData)) {
    S_O_S(MSG_LOG_DATA_SIZE, LOG_DATA_TYPE_MESSAGE, 0);
  }
  LogMessageData data;
  data.id = id;
  data.params[0] = param1;
  data.params[1] = param2;
  LogEntry e = addLogEntry(LOG_DATA_TYPE_MESSAGE, (LogData *) &data);
  #ifdef DEBUG_LOG
    Serial.println(F("DEBUG_LOG: logMessage(..)"));
  #endif
  return e.timestamp;
}

Timestamp Log::logValues(Temperature water, Temperature ambient, Flags flags) {
  if (sizeof(LogValuesData) != sizeof(LogData)) {
    S_O_S(MSG_LOG_DATA_SIZE, LOG_DATA_TYPE_VALUES, 0);
  }
  LogValuesData data;
  data.water = water;
  data.ambient = ambient;
  data.flags = flags;
  LogEntry e = addLogEntry(LOG_DATA_TYPE_VALUES, (LogData *) &data);
  #ifdef DEBUG_LOG
    Serial.println(F("DEBUG_LOG: logValues(..)"));
  #endif
  return e.timestamp;
}

Timestamp Log::logState(StateID previous, StateID current, EventID event) {
  if (sizeof(LogStateData) != sizeof(LogData)) {
    S_O_S(MSG_LOG_DATA_SIZE, LOG_DATA_TYPE_STATE, 0);
  }
  LogStateData data;
  data.previous = previous;
  data.current = current;
  data.event = event;
  LogEntry e = addLogEntry(LOG_DATA_TYPE_STATE, (LogData *) &data);
  #ifdef DEBUG_LOG
    Serial.println(F("DEBUG_LOG: logState(..)"));
  #endif
  return e.timestamp;
}

Timestamp Log::logConfigParam(ConfigParamID id, float newValue) {
  if (sizeof(LogValuesData) != sizeof(LogData)) {
    S_O_S(MSG_LOG_DATA_SIZE, LOG_DATA_TYPE_CONFIG, 0);
  }
  LogConfigParamData data;
  data.id = id;
  data.newValue = newValue;
  LogEntry e = addLogEntry(LOG_DATA_TYPE_CONFIG, (LogData *) &data);
  #ifdef DEBUG_LOG
    Serial.println(F("DEBUG_LOG: logConfigParam(..)"));
  #endif
  return e.timestamp;
}

