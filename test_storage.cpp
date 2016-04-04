#include "unit_test.h"
#ifdef TEST_STORAGE
  #line 4 "test_storage.cpp"
  #include <ArduinoUnit.h>
  #include "storage.h"

  
  
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
    assertEqual(t1>>ID_BITS, sec); // check same second
    assertEqual(t1 & 0xF, 0L); // check identity counter
  
    // 1 .. 15
    Timestamp t2;
    for(unsigned long i=1; i<=15; i++) {
      t2 = timestamp();
      assertMore(t2, t1);
      assertEqual(t2>>ID_BITS, sec);
      assertEqual(t2 & 0xF, i);
      t1 = t2;
    }
    // 16
    t1 = timestamp();
    // ensure we are in the next full second now
    assertEqual(t1>>ID_BITS, sec+1); 
    assertEqual(t1 & 0xF, 0L);
    
    // 17
    t1 = timestamp();
    assertEqual(t1>>ID_BITS, sec+1);
    assertEqual(t1 & 0xF, 1L);
  }
  
  
  test(storage_config) {
    Storage storage = Storage();
    
    storage.clearConfigParams();
    assertEqual(storage.version(), 0L);
    
    ConfigParams c;
    storage.getConfigParams(&c);
    assertNotEqual(storage.version(), 0L);
    assertEqual(c.targetTemp, DEFAULT_TARGET_TEMP);
    
    c.targetTemp = 40.0;
    storage.updateConfigParams(&c);
    ConfigParams c1;
    storage.getConfigParams(&c1);
    assertEqual(c1.targetTemp, 40.0);
    
    storage.clearConfigParams();
    assertEqual(storage.version(), 0L);
    ConfigParams c2;
    storage.readConfigParams(&c2);
    assertEqual(c2.targetTemp, 0.0);
  }
  
  
  test(storage_log) {
    Storage storage = Storage();

    storage.resetLog();
    assertEqual(storage.maxLogEntries(), UNIT_TEST_LOG_ENTRIES);
    assertEqual(storage.currentLogEntries(), 1);
    assertEqual(storage.logTailIndex(), 0);
    assertEqual(storage.logHeadIndex(), 1);
  
    // Test ring buffer logging:
    storage.logValues(30.0, 20.0, 0);
    assertEqual(storage.currentLogEntries(), 2);
    assertEqual(storage.logTailIndex(), 0);
    assertEqual(storage.logHeadIndex(), 2);
    
    storage.logValues(31.0, 20.0, 0);
    assertEqual(storage.currentLogEntries(), 3);
    assertEqual(storage.logTailIndex(), 0);
    assertEqual(storage.logHeadIndex(), 3);
    
    storage.logValues(32.0, 20.0, 0);
    assertEqual(storage.currentLogEntries(), 4);
    assertEqual(storage.logTailIndex(), 0);
    assertEqual(storage.logHeadIndex(), 4);
    
    storage.logValues(33.0, 20.0, 0);
    assertEqual(storage.currentLogEntries(), 4);
    assertEqual(storage.logTailIndex(), 1);
    assertEqual(storage.logHeadIndex(), 0);
    
    storage.logValues(34.0, 20.0, 0);
    assertEqual(storage.currentLogEntries(), 4);
    assertEqual(storage.logTailIndex(), 2);
    assertEqual(storage.logHeadIndex(), 1);
    
    storage.logValues(35.0, 20.0, 0);
    assertEqual(storage.currentLogEntries(), 4);
    assertEqual(storage.logTailIndex(), 3);
    assertEqual(storage.logHeadIndex(), 2);
  
    // Test initialisation:
    storage.resetLog();
    storage.initLog();
    assertEqual(storage.logTailIndex(), 0);
    assertEqual(storage.logHeadIndex(), 1);
    
    storage.logValues(30.0, 20.0, 0);
    storage.initLog();
    assertEqual(storage.logTailIndex(), 0);
    assertEqual(storage.logHeadIndex(), 2);
    
    storage.logValues(31.0, 20.0, 0);
    storage.initLog();
    assertEqual(storage.logTailIndex(), 0);
    assertEqual(storage.logHeadIndex(), 3);
    
    storage.logValues(32.0, 20.0, 0);
    storage.initLog();
    assertEqual(storage.logTailIndex(), 0);
    assertEqual(storage.logHeadIndex(), 4);
    
    storage.logValues(33.0, 20.0, 0);
    storage.initLog();
    assertEqual(storage.logTailIndex(), 1);
    assertEqual(storage.logHeadIndex(), 0);
    
    storage.logValues(34.0, 20.0, 0);
    storage.initLog();
    assertEqual(storage.logTailIndex(), 2);
    assertEqual(storage.logHeadIndex(), 1);
    
    storage.logValues(35.0, 20.0, 0);
    storage.initLog();
    assertEqual(storage.logTailIndex(), 3);
    assertEqual(storage.logHeadIndex(), 2);
  }
#endif

