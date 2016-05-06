#include <assert.h>
#include "log.h"

// #define DEBUG_LOG

unsigned long timeBase_sec = 0L;

void adjustLogTime(Timestamp mostRecent) {
  unsigned long mostRecent_sec = mostRecent >> TIMESTAMP_ID_BITS;
  unsigned long current_sec = millis() / 1000L;
  if (mostRecent_sec >= current_sec) {
    timeBase_sec = mostRecent_sec;
  } else {
    resetLogTime();
  }
}

void resetLogTime() {
  timeBase_sec = 0L;
}

LogTimeRaw logTime() {
  long ms = millis();
  LogTimeRaw t = {timeBase_sec + (ms / 1000L), (unsigned short) (ms % 1000L)};
  return t;
}


Timestamp timestamp() {
  static unsigned long last_sec = 0L;
  static byte count = 0;
  LogTimeRaw t = logTime();
  
  if (t.sec > last_sec) {
    count = 0;
    last_sec = t.sec;
  } else {
    count++;
    if (count == 16) {
      // wait for the next full second to start (with an added safety margin of 1):
      delay(1000 - t.ms + 1);
      t = logTime();
      assert(t.sec > last_sec);
      count = 0;
      last_sec = t.sec;
    }
  }
  #ifdef DEBUG_LOG
    Serial.print("DEBUG_LOG: Timestamp ");
    Serial.print(t.sec);
    Serial.print(".");
    Serial.println(count);
  #endif
  return t.sec << TIMESTAMP_ID_BITS | count;
}


LogEntry createLogEntry(LogTypeID type, LogData data) {
  LogEntry entry;
  entry.timestamp = timestamp();
  entry.type = type;
  #ifdef DEBUG_LOG
    Serial.print("DEBUG_LOG: createLogEntry: type ");
    Serial.print(entry.type);
    Serial.print(" => timestamp: ");
    Serial.println(entry.timestamp);
  #endif
  memcpy(&(entry.data), &data, sizeof(LogData));
  return entry;
}


LogEntry createLogValuesEntry(Temperature water, Temperature ambient, Flags flags) {
  LogValuesData data;
  data.water = water;
  data.ambient = ambient;
  data.flags = flags;

  // copy data to generic log-data type:
  LogData generic;
  assert(sizeof(LogValuesData) == sizeof(LogData));
  memcpy(&generic, &data, sizeof(LogValuesData));
  
  return createLogEntry(LOG_VALUES, generic);
}

LogEntry createLogStateEntry(StateID previous, StateID current, EventID event) {
  LogStateData data;
  data.previous = previous;
  data.current = current;
  data.event = event;

  // copy data to generic log-data type:
  LogData generic;
  assert(sizeof(LogStateData) == sizeof(LogData));
  memcpy(&generic, &data, sizeof(LogStateData));
  
  return createLogEntry(LOG_STATE, generic);
}

LogEntry createLogMessageEntry(MessageID id, short param1, short param2) {
  LogMessageData data;
  data.id = id;
  data.params[0] = param1;
  data.params[1] = param2;

  // copy data to generic log-data type:
  LogData generic;
  assert(sizeof(LogMessageData) == sizeof(LogData));
  memcpy(&generic, &data, sizeof(LogMessageData));
  
  return createLogEntry(LOG_MESSAGE, generic);
}

