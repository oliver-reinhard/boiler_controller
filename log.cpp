#include <assert.h>
#include "log.h"

//#define DEBUG_LOG

#define ASCII_0 48  // char(48)

/*
 * The value (in seconds) added to Arduino board millis().
 */
uint32_t timeBase_sec = 0L;

/*
 * The (adjusted) seconds of time when a the last timestamp was issued.
 */
static uint32_t last_sec = 0L;

/*
 * The ID count of the last issued timestamp.
 */
static uint8_t count = 0;


void adjustLogTime(Timestamp mostRecent) {
  uint32_t mostRecent_sec = mostRecent >> TIMESTAMP_ID_BITS;
  uint32_t current_sec = millis() / 1000L;
  if (mostRecent_sec >= current_sec) {
    timeBase_sec = mostRecent_sec + 1; // continue at the "next" second
  } else {
    resetLogTime();
  }
}

void resetLogTime() {
  timeBase_sec = 0L;
  last_sec = 0L;
  count = 0;
}

LogTimeRaw logTime() {
  uint32_t ms = millis();
  LogTimeRaw t = {timeBase_sec + (ms / 1000L), (uint16_t) (ms % 1000L)};
  return t;
}


Timestamp timestamp() {
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
  Timestamp ts = t.sec << TIMESTAMP_ID_BITS | count;
  #ifdef DEBUG_LOG
    Serial.print(F("DEBUG_LOG: Timestamp "));
    Serial.println(formatTimestamp(ts));
  #endif
  return ts;
}


String formatTimestamp(Timestamp t) {
  char s[13];
  uint32_t sec = t >> TIMESTAMP_ID_BITS;
  uint8_t count = t % 16;
  s[12] = '\0';
  s[11] = ASCII_0 + count % 10;
  s[10] = ASCII_0 + count / 10;
  s[9] = '.';
  for(int16_t i = 8; i >= 0; i--) {
    s[i] = ASCII_0 + sec % 10;
    sec /= 10;
  }
  return s;
}
 
/*
 * Generic log-entry creation.
 */
LogEntry createLogEntry(LogTypeID type, LogData data) {
  LogEntry entry;
  entry.timestamp = timestamp();
  entry.type = type;
  #ifdef DEBUG_LOG
    Serial.print(F("DEBUG_LOG: createLogEntry: type "));
    Serial.print(entry.type);
    Serial.print(F(" => timestamp: "));
    Serial.println(formatTimestamp(entry.timestamp));
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

LogEntry createLogMessageEntry(MessageEnum id, int16_t param1, int16_t param2) {
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

LogEntry createConfigParamEntry(ConfigParamID id, float newValue) {
  LogConfigParamData data;
  data.id = id;
  data.newValue = newValue;

  // copy data to generic log-data type:
  LogData generic;
  assert(sizeof(LogConfigParamData) == sizeof(LogData));
  memcpy(&generic, &data, sizeof(LogConfigParamData));
  
  return createLogEntry(LOG_CONFIG, generic);
}

