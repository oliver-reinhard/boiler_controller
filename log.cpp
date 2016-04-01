#include <assert.h>
#include "log.h"

//#define DEBUG_LOG

unsigned long timeBase_sec = 0L;

void adjustLogTime(Timestamp mostRecent) {
  unsigned long mostRecent_sec = mostRecent >> ID_BITS;
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
  Serial.print("Timestamp ");
  Serial.print(t.sec);
  Serial.print(".");
  Serial.println(count);
  #endif
  return t.sec << ID_BITS | count;
}


LogEntry createLogEntry(LogType type, LogData data) {
  LogEntry entry;
  entry.timestamp = timestamp();
  entry.type = type;
  memcpy(&(entry.data), &data, sizeof(LogData));
  return entry;
}


LogEntry createLogValuesEntry(Temperature water, Temperature ambient, Flags flags) {
  LogValuesData values;
  values.water = water;
  values.ambient = ambient;
  values.flags = flags;

  // copy data to generic log-data type:
  LogData data;
  assert(sizeof(LogValuesData) == sizeof(LogData));
  memcpy(&data, &values, sizeof(LogValuesData));
  
  return createLogEntry(LOG_VALUES, data);
}

LogEntry createLogMessageEntry(MessageID id, short param1, short param2) {
  LogMessageData values;
  values.id = id;
  values.params[0] = param1;
  values.params[1] = param2;

  // copy data to generic log-data type:
  LogData data;
  assert(sizeof(LogMessageData) == sizeof(LogData));
  memcpy(&data, &values, sizeof(LogMessageData));
  
  return createLogEntry(LOG_MESSAGE, data);
}

