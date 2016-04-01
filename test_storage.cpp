#include "unit_test.h"
#ifdef TEST_STORAGE
  #line 4 "test_storage.cpp"
  #include <ArduinoUnit.h>
  #include "Arduino.h"
  #include "storage.h"
  
  test(timestamp) {
    // ensure this test is not run within the first second of Arduino board time:
    delay(1000);
    unsigned long ms = millis();
    if (ms % 1000 > 800) {
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
  
  /*
  test(config) {
    clearConfig();
    assertEqual(version(), 0L);
    
    Config c0;
    readConfig(&c0);
    assertEqual(c0.targetTemp, 0.0);
  
    Config c;
    initConfig(&c);
    assertNotEqual(version(), 0L);
    assertEqual(c.targetTemp, DEFAULT_TARGET_TEMP);
    
    c.targetTemp = 40.0;
    updateConfig(&c);
    Config c1;
    readConfig(&c1);
    assertEqual(c1.targetTemp, 40.0);
    
    clearConfig();
    assertEqual(version(), 0L);
    Config c2;
    readConfig(&c2);
    assertEqual(c2.targetTemp, 0.0);
  }
  
  */
  test(log) {
    resetLog();
    assertEqual(maxLogEntries(), UNIT_TEST_LOG_ENTRIES);
    assertEqual(currentLogEntries(), 1);
    assertEqual(logTailIndex(), 0);
    assertEqual(logHeadIndex(), 1);
  
    // Test ring buffer logging:
    logValues(30.0, 20.0, 0);
    assertEqual(currentLogEntries(), 2);
    assertEqual(logTailIndex(), 0);
    assertEqual(logHeadIndex(), 2);
    
    logValues(31.0, 20.0, 0);
    assertEqual(currentLogEntries(), 3);
    assertEqual(logTailIndex(), 0);
    assertEqual(logHeadIndex(), 3);
    
    logValues(32.0, 20.0, 0);
    assertEqual(currentLogEntries(), 4);
    assertEqual(logTailIndex(), 0);
    assertEqual(logHeadIndex(), 4);
    
    logValues(33.0, 20.0, 0);
    assertEqual(currentLogEntries(), 4);
    assertEqual(logTailIndex(), 1);
    assertEqual(logHeadIndex(), 0);
    
    logValues(34.0, 20.0, 0);
    assertEqual(currentLogEntries(), 4);
    assertEqual(logTailIndex(), 2);
    assertEqual(logHeadIndex(), 1);
    
    logValues(35.0, 20.0, 0);
    assertEqual(currentLogEntries(), 4);
    assertEqual(logTailIndex(), 3);
    assertEqual(logHeadIndex(), 2);
  
    // Test initialisation:
    resetLog();
    initLog();
    assertEqual(logTailIndex(), 0);
    assertEqual(logHeadIndex(), 1);
    
    logValues(30.0, 20.0, 0);
    initLog();
    assertEqual(logTailIndex(), 0);
    assertEqual(logHeadIndex(), 2);
    
    logValues(31.0, 20.0, 0);
    initLog();
    assertEqual(logTailIndex(), 0);
    assertEqual(logHeadIndex(), 3);
    
    logValues(32.0, 20.0, 0);
    initLog();
    assertEqual(logTailIndex(), 0);
    assertEqual(logHeadIndex(), 4);
    
    logValues(33.0, 20.0, 0);
    initLog();
    assertEqual(logTailIndex(), 1);
    assertEqual(logHeadIndex(), 0);
    
    logValues(34.0, 20.0, 0);
    initLog();
    assertEqual(logTailIndex(), 2);
    assertEqual(logHeadIndex(), 1);
    
    logValues(35.0, 20.0, 0);
    initLog();
    assertEqual(logTailIndex(), 3);
    assertEqual(logHeadIndex(), 2);
  }
#endif

