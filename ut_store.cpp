#include "bc_setup.h"
#ifdef UT_STORE
  #line 4 "ut_store.cpp"
  #include <ArduinoUnit.h>
  #include "store.h"

  //#define DEBUG_UT_STORE
  test(storage_timestamp) {
    // ensure this test is not run within the first second of Arduino board time:
    delay(1000);
    unsigned long ms = millis();
    if (ms % 1000 > 500) {
      // sleep until the next full second has started
      delay(1000 - (ms % 1000) + 1);
      ms = millis();
      assertLessOrEqual(ms % 1000, 3);
    }
    unsigned long sec = ms / 1000;
    // 0
    Timestamp t1 = timestamp();
    assertEqual(t1>>TIMESTAMP_ID_BITS, sec); // check same second
    assertEqual(t1 & 0xF, 0L); // check identity counter
  
    // 1 .. 15
    Timestamp t2;
    for(unsigned long i=1; i<=15; i++) {
      t2 = timestamp();
      assertMore(t2, t1);
      assertEqual(t2>>TIMESTAMP_ID_BITS, sec);
      assertEqual(t2 & 0xF, i);
      t1 = t2;
    }
    // 16
    t1 = timestamp();
    // ensure we are in the next full second now
    assertEqual(t1>>TIMESTAMP_ID_BITS, sec+1); 
    assertEqual(t1 & 0xF, 0L);
    
    // 17
    t1 = timestamp();
    assertEqual(t1>>TIMESTAMP_ID_BITS, sec+1);
    assertEqual(t1 & 0xF, 1L);
  }
  
  
  test(storage_config) {
    Storage storage = Storage();
    
    storage.clearConfigParams();
    assertEqual(storage.version(), 0L);
    
    ConfigParams c;
    storage.getConfigParams(&c);
    #ifdef DEBUG_UT_STORE
      printConfig(c);
    #endif
    assertNotEqual(storage.version(), 0L);
    assertEqual(c.targetTemp, DEFAULT_TARGET_TEMP);
    assertEqual(c.heaterCutOutWaterTemp, DEFAULT_HEATER_CUT_OUT_WATER_TEMP); 
    
    c.targetTemp = 4000;
    storage.updateConfigParams(&c);
    ConfigParams c1;
    storage.getConfigParams(&c1);
    assertEqual(c1.targetTemp, 4000);
    
    storage.clearConfigParams();
    assertEqual(storage.version(), 0L);
    ConfigParams c2;
    storage.readConfigParams(&c2);
    assertEqual(c2.targetTemp, 0);
  }
  
  
  test(storage_log) {
    Storage storage = Storage();

    storage.resetLog();
    assertEqual(storage.maxLogEntries(), UNIT_TEST_LOG_ENTRIES - 1);
    assertEqual(storage.currentLogEntries(), 1);
    assertEqual(storage.logTailIndex(), 0);
    assertEqual(storage.logHeadIndex(), 1);
  
    // Test ring buffer logging:
    storage.logValues(3000, 2000, 0);
    assertEqual(storage.currentLogEntries(), 2);
    assertEqual(storage.logTailIndex(), 0);
    assertEqual(storage.logHeadIndex(), 2);
    
    storage.logValues(3100, 2000, 0);
    assertEqual(storage.currentLogEntries(), 3);
    assertEqual(storage.logTailIndex(), 0);
    assertEqual(storage.logHeadIndex(), 3);
    
    storage.logValues(3200, 2000, 0);
    assertEqual(storage.currentLogEntries(), 4);
    assertEqual(storage.logTailIndex(), 0);
    assertEqual(storage.logHeadIndex(), 4);
    
    storage.logValues(3300, 2000, 0);
    assertEqual(storage.currentLogEntries(), 4);
    assertEqual(storage.logTailIndex(), 1);
    assertEqual(storage.logHeadIndex(), 0);
    
    storage.logValues(3400, 2000, 0);
    assertEqual(storage.currentLogEntries(), 4);
    assertEqual(storage.logTailIndex(), 2);
    assertEqual(storage.logHeadIndex(), 1);
    
    storage.logValues(3500, 2000, 0);
    assertEqual(storage.currentLogEntries(), 4);
    assertEqual(storage.logTailIndex(), 3);
    assertEqual(storage.logHeadIndex(), 2);
  
    // Test initialisation:
    storage.resetLog();
    storage.initLog();
    assertEqual(storage.logTailIndex(), 0);
    assertEqual(storage.logHeadIndex(), 1);
    
    storage.logValues(3000, 2000, 0);
    storage.initLog();
    assertEqual(storage.logTailIndex(), 0);
    assertEqual(storage.logHeadIndex(), 2);
    
    storage.logValues(3100, 2000, 0);
    storage.initLog();
    assertEqual(storage.logTailIndex(), 0);
    assertEqual(storage.logHeadIndex(), 3);
    
    storage.logValues(3200, 2000, 0);
    storage.initLog();
    assertEqual(storage.logTailIndex(), 0);
    assertEqual(storage.logHeadIndex(), 4);
    
    storage.logValues(3300, 2000, 0);
    storage.initLog();
    assertEqual(storage.logTailIndex(), 1);
    assertEqual(storage.logHeadIndex(), 0);
    
    storage.logValues(3400, 2000, 0);
    storage.initLog();
    assertEqual(storage.logTailIndex(), 2);
    assertEqual(storage.logHeadIndex(), 1);
    
    storage.logValues(3500, 2000, 0);
    storage.initLog();
    assertEqual(storage.logTailIndex(), 3);
    assertEqual(storage.logHeadIndex(), 2);
  }
  
  test(storage_log_reader) {
    Storage storage = Storage();

    storage.resetLog(); // creates a first log entry
    storage.logValues(3000, 2000, 0);
    storage.logValues(3100, 2000, 0);
    storage.logValues(3200, 2000, 0);

    storage.readMostRecentLogEntries(0);
    LogReader *r = storage.getLogReader();
    assertEqual(r->toRead, 4);
    LogEntry e;
    //
    assertTrue(storage.nextLogEntry(&e));
    assertEqual(int(e.type), int(LOG_VALUES));
    LogValuesData lvd1;
    memcpy(&lvd1, &(e.data), sizeof(LogValuesData));
    assertEqual(lvd1.water, 3200);
    //
    assertTrue(storage.nextLogEntry(&e));
    assertEqual(int(e.type), int(LOG_VALUES));
    LogValuesData lvd2;
    memcpy(&lvd2, &(e.data), sizeof(LogValuesData));
    assertEqual(lvd2.water, 3100);
    //
    assertTrue(storage.nextLogEntry(&e));
    assertEqual(int(e.type), int(LOG_VALUES));
    LogValuesData lvd3;
    memcpy(&lvd3, &(e.data), sizeof(LogValuesData));
    assertEqual(lvd3.water, 3000);
    //
    assertTrue(storage.nextLogEntry(&e));
    assertEqual(int(e.type), int(LOG_MESSAGE));
    LogMessageData lmd;
    memcpy(&lmd, &(e.data), sizeof(LogMessageData));
    assertEqual(lmd.id, 1); // MSG_LOG_INIT
    //
    assertFalse(storage.nextLogEntry(&e));
  }
    
#endif

