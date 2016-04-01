#include "unit_test.h"
#ifdef TEST_STATE
  #line 4 "test_state.cpp"
  #include <ArduinoUnit.h>
  #include "Arduino.h"
  #include "storage.h"
  #include "state.h"
  #include "test_state.h"

  /*
   * Class MockControlActions
   */
  unsigned short MockControlActions::totalInvocations() {
    return 
      heatTrueCount +
      heatFalseCount +
      logValuesTrueCount +
      logValuesFalseCount +
      setConfigParamCount +
      getLogCount +
      getConfigCount +
      getStatCount;
  }
   
  void MockControlActions::resetCounters() {
    heatTrueCount = 0;
    heatFalseCount = 0;
    logValuesTrueCount = 0;
    logValuesFalseCount = 0;
    setConfigParamCount = 0;
    getLogCount = 0;
    getConfigCount = 0;
    getStatCount = 0;
  }
      
  void MockControlActions::readSensors(OperationalParams *op) {
    if (op == NULL) { } // prevent warning "unused parameter ..."
  }

  void MockControlActions::heat(boolean on, OperationalParams *op) {
    op->heating = on;
    if (on) {
      heatTrueCount++;
    } else {
      heatFalseCount++;
    }
  }

  void MockControlActions::logValues(boolean on, OperationalParams *op) {
    op->loggingValues = on;
    if (on) {
      logValuesTrueCount++;
    } else {
      logValuesFalseCount++;
    }
  }

  void MockControlActions::setConfigParam() {
    setConfigParamCount++;
  }
  
  void MockControlActions::getLog() {
    getLogCount++;
  }
  
  void MockControlActions::getConfig() {
    getConfigCount++;
  }
  
  void MockControlActions::getStat() {
    getStatCount++;
  }


  /*
   * Tests
   */
  test(stateInit) {
    ConfigParams config;
    boolean updated;
    initConfigParams(&config, &updated);
    
    OperationalParams op;
    MockControlActions control;

    ExecutionContext context;
    context.config = &config;
    context.op = &op;
    context.control = &control;

    BoilerStateAutomaton automaton = BoilerStateAutomaton(&context);
    
  }
  
#endif

