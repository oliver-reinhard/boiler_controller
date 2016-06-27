#line 2 "ut_Logging.h"
#include <ArduinoUnit.h>

#include "LogTime.h"
#include "Logging.h"

//#define DEBUG_UT_LOGGING

#define UNIT_TEST_LOG_OFFSET 40  // EEPROM offset (bytes)
#define UNIT_TEST_LOG_ENTRIES 5  // numer of LogEntry slots

typedef enum {
  LOG_TYPE_MESSAGE = 0,
  LOG_TYPE_VALUES = 1
} LogTypeEnum;

struct LogMessageData {
  MessageID id;
  int16_t   params[2];
};

struct LogValuesData {
  int16_t value;
  byte    filler[3];
};

class TestLogging : public AbstractLog {
  public:
    TestLogging() : AbstractLog(UNIT_TEST_LOG_OFFSET, sizeof(uint16_t) + UNIT_TEST_LOG_ENTRIES * sizeof(LogEntry)) { };  // uint16_t = number of logEntries
  
    Timestamp logMessage(MessageID id, int16_t param1, int16_t param2);
    Timestamp logValues(int16_t value);
};

Timestamp TestLogging::logMessage(MessageID id, int16_t param1, int16_t param2) {
  LogMessageData data;
  data.id = id;
  data.params[0] = param1;
  data.params[1] = param2;
  LogData generic;
  memcpy(&generic, &data, sizeof(LogMessageData));
  LogEntry e = addLogEntry(LOG_TYPE_MESSAGE, &generic);
  return e.timestamp;
}

Timestamp TestLogging::logValues(int16_t value) {
  LogValuesData data;
  data.value = value;
  LogData generic;
  memcpy(&generic, &data, sizeof(LogValuesData));
  LogEntry e = addLogEntry(LOG_TYPE_VALUES, &generic);
  return e.timestamp;
}

/*
 * TEST CASES
 *

test(log_timestamp) {
  // ensure this test is not run within the first second of Arduino board time:
  delay(1000);
  uint32_t ms = millis();
  if (ms % 1000 > 500) {
    // sleep until the next full second has started
    delay(1000 - (ms % 1000) + 1);
    ms = millis();
    assertLessOrEqual(ms % 1000, 3);
  }
  uint32_t sec = ms / 1000;
  // 0
  LogTime lt = LogTime();
  Timestamp t1 = lt.timestamp();
  assertEqual(t1>>TIMESTAMP_ID_BITS, sec); // check same second
  assertEqual(t1 & 0xF, 0L); // check identity counter

  // 1 .. 15
  Timestamp t2;
  for(uint32_t i=1; i<=15; i++) {
    t2 = lt.timestamp();
    assertMore(t2, t1);
    assertEqual(t2>>TIMESTAMP_ID_BITS, sec);
    assertEqual(t2 & 0xF, i);
    t1 = t2;
  }
  // 16
  t1 = lt.timestamp();
  // ensure we are in the next full second now
  assertEqual(t1>>TIMESTAMP_ID_BITS, sec+1); 
  assertEqual(t1 & 0xF, 0L);
  
  // 17
  t1 = lt.timestamp();
  assertEqual(t1>>TIMESTAMP_ID_BITS, sec+1);
  assertEqual(t1 & 0xF, 1L);
}


test(log_ring_buffer) {
  TestLogging logging = TestLogging();

  logging.resetLog();
  assertEqual(logging.maxLogEntries(), UNIT_TEST_LOG_ENTRIES - 1);
  assertEqual(logging.currentLogEntries(), 1);
  assertEqual(logging.logTailIndex, 0);
  assertEqual(logging.logHeadIndex, 1);

  // Test ring buffer logging:
  logging.logValues(3000);
  assertEqual(logging.currentLogEntries(), 2);
  assertEqual(logging.logTailIndex, 0);
  assertEqual(logging.logHeadIndex, 2);
  
  logging.logValues(3100);
  assertEqual(logging.currentLogEntries(), 3);
  assertEqual(logging.logTailIndex, 0);
  assertEqual(logging.logHeadIndex, 3);
  
  logging.logValues(3200);
  assertEqual(logging.currentLogEntries(), 4);
  assertEqual(logging.logTailIndex, 0);
  assertEqual(logging.logHeadIndex, 4);
  
  logging.logValues(3300);
  assertEqual(logging.currentLogEntries(), 4);
  assertEqual(logging.logTailIndex, 1);
  assertEqual(logging.logHeadIndex, 0);
  
  logging.logValues(3400);
  assertEqual(logging.currentLogEntries(), 4);
  assertEqual(logging.logTailIndex, 2);
  assertEqual(logging.logHeadIndex, 1);
  
  logging.logValues(3500);
  assertEqual(logging.currentLogEntries(), 4);
  assertEqual(logging.logTailIndex, 3);
  assertEqual(logging.logHeadIndex, 2);
}

test(log_init) {
  TestLogging logging = TestLogging();

  // Test initialisation:
  logging.resetLog();
  logging.initLog();
  assertEqual(logging.logTailIndex, 0);
  assertEqual(logging.logHeadIndex, 1);
  
  logging.logValues(3000);
  logging.initLog();
  assertEqual(logging.logTailIndex, 0);
  assertEqual(logging.logHeadIndex, 2);
  
  logging.logValues(3100);
  logging.initLog();
  assertEqual(logging.logTailIndex, 0);
  assertEqual(logging.logHeadIndex, 3);
  
  logging.logValues(3200);
  logging.initLog();
  assertEqual(logging.logTailIndex, 0);
  assertEqual(logging.logHeadIndex, 4);
  
  logging.logValues(3300);
  logging.initLog();
  assertEqual(logging.logTailIndex, 1);
  assertEqual(logging.logHeadIndex, 0);
  
  logging.logValues(3400);
  logging.initLog();
  assertEqual(logging.logTailIndex, 2);
  assertEqual(logging.logHeadIndex, 1);
  
  logging.logValues(3500);
  logging.initLog();
  assertEqual(logging.logTailIndex, 3);
  assertEqual(logging.logHeadIndex, 2);
}

test(log_reader_unnotified) {
  TestLogging logging = TestLogging();

  logging.resetLog(); // creates a first log entry
  logging.logValues(3000);
  logging.logValues(3100);
  logging.logValues(3200);
  
  logging.readUnnotifiedLogEntries(); // returns oldest first, then --> newer
  assertEqual(logging.reader.toRead, 4);
  LogEntry e;
  //
  assertTrue(logging.nextLogEntry(&e));
  assertEqual(int(e.type), int(LOG_TYPE_MESSAGE));
  LogMessageData lmd;
  memcpy(&lmd, &(e.data), sizeof(LogMessageData));
  assertEqual(lmd.id, 1); // MSG_LOG_INIT
  //
  assertTrue(logging.nextLogEntry(&e));
  assertEqual(int(e.type), int(LOG_TYPE_VALUES));
  LogValuesData lvd1;
  memcpy(&lvd1, &(e.data), sizeof(LogValuesData));
  assertEqual(lvd1.value, 3000);
  //
  // Stop with current reader, create a new reader
  //
  logging.readUnnotifiedLogEntries(); // returns oldest first, then --> newer
  assertEqual(logging.reader.toRead, 2);
  //
  assertTrue(logging.nextLogEntry(&e));
  assertEqual(int(e.type), int(LOG_TYPE_VALUES));
  LogValuesData lvd2;
  memcpy(&lvd2, &(e.data), sizeof(LogValuesData));
  assertEqual(lvd2.value, 3100);
  //
  assertTrue(logging.nextLogEntry(&e));
  assertEqual(int(e.type), int(LOG_TYPE_VALUES));
  LogValuesData lvd3;
  memcpy(&lvd3, &(e.data), sizeof(LogValuesData));
  assertEqual(lvd3.value, 3200);
}

test(log_reader_most_recent) {
  TestLogging logging = TestLogging();

  logging.resetLog(); // creates a first log entry
  logging.logValues(3000);
  logging.logValues(3100);
  logging.logValues(3200);
  
  logging.readMostRecentLogEntries(0); // returns most recent first, then --> older
  assertEqual(logging.reader.toRead, 4);
  LogEntry e;
  //
  assertTrue(logging.nextLogEntry(&e));
  assertEqual(int(e.type), int(LOG_TYPE_VALUES));
  LogValuesData lvd1;
  memcpy(&lvd1, &(e.data), sizeof(LogValuesData));
  assertEqual(lvd1.value, 3200);
  //
  assertTrue(logging.nextLogEntry(&e));
  assertEqual(int(e.type), int(LOG_TYPE_VALUES));
  LogValuesData lvd2;
  memcpy(&lvd2, &(e.data), sizeof(LogValuesData));
  assertEqual(lvd2.value, 3100);
  //
  assertTrue(logging.nextLogEntry(&e));
  assertEqual(int(e.type), int(LOG_TYPE_VALUES));
  LogValuesData lvd3;
  memcpy(&lvd3, &(e.data), sizeof(LogValuesData));
  assertEqual(lvd3.value, 3000);
  //
  assertTrue(logging.nextLogEntry(&e));
  assertEqual(int(e.type), int(LOG_TYPE_MESSAGE));
  LogMessageData lmd;
  memcpy(&lmd, &(e.data), sizeof(LogMessageData));
  assertEqual(lmd.id, 1); // MSG_LOG_INIT
  //
  assertFalse(logging.nextLogEntry(&e));
}
*/
