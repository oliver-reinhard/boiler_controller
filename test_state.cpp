#include "unit_test.h"
#ifdef TEST_STATE
  #line 4 "test_state.cpp"
  #include <ArduinoUnit.h>
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

    assertEqual(automaton.state()->id(), STATE_INIT);
    assertEqual(control.totalInvocations(), 0);

    // water sensor NOT OK YET => evaluate => no event
    EventCandidates cand = automaton.evaluate();
    assertEqual(int(cand), int(EVENT_NONE));
    assertEqual(automaton.state()->id(), STATE_INIT);
    assertEqual(control.totalInvocations(), 0);

    // transition (INVALID event) => stay in state INIT
    automaton.transition(EVENT_SET_CONFIG); 
    assertEqual(automaton.state()->id(), STATE_INIT);
    assertEqual(control.totalInvocations(), 0);

    // test transition again, must not log again
    automaton.transition(EVENT_SET_CONFIG); 

    //
    // set water sensor OK => evaluate => event READY
    //
    op.water.sensorStatus = SENSOR_OK;
    op.water.currentTemp = 2300;
    cand = automaton.evaluate();
    assertEqual(int(cand), int(EVENT_READY));
    assertEqual(automaton.state()->id(), STATE_INIT);
    assertEqual(control.totalInvocations(), 0);

    // transition (event READY) => state IDLE
    automaton.transition(EVENT_READY);
    assertEqual(automaton.state()->id(), STATE_IDLE);
    assertEqual(control.totalInvocations(), 0);
    assertEqual(int(op.loggingValues), int(false));
    assertEqual(int(op.heating), int(false));

    //
    // user command SET_CONFIG
    //
    op.userCommands = CMD_SET_CONFIG;
    cand = automaton.evaluate();
    assertEqual(int(cand), int(EVENT_SET_CONFIG));
    op.userCommands = CMD_NONE;
    
    // transition (event SET_CONFIG) => stay in state IDLE
    automaton.transition(EVENT_SET_CONFIG); 
    assertEqual(automaton.state()->id(), STATE_IDLE);
    assertEqual(control.setConfigParamCount, 1);
    assertEqual(control.totalInvocations(), 1);
    control.resetCounters();

    //
    // user command REC ON
    //
    op.userCommands = CMD_REC_ON;
    cand = automaton.evaluate();
    assertEqual(int(cand), int(EVENT_REC_ON));
    op.userCommands = CMD_NONE;
    
    // transition (event REC_ON) => state STANDBY
    automaton.transition(EVENT_REC_ON); 
    assertEqual(automaton.state()->id(), STATE_STANDBY);
    assertEqual(control.logValuesTrueCount, 1);
    assertEqual(control.totalInvocations(), 1);
    assertEqual(int(op.loggingValues), int(true));
    assertEqual(int(op.heating), int(false));
    control.resetCounters();

    //
    // user command REC OFF
    //
    op.userCommands = CMD_REC_OFF;
    cand = automaton.evaluate();
    assertEqual(int(cand), int(EVENT_REC_OFF));
    op.userCommands = CMD_NONE;
    
    // transition (event REC_OFF) => state IDLE
    automaton.transition(EVENT_REC_OFF); 
    assertEqual(automaton.state()->id(), STATE_IDLE);
    assertEqual(control.logValuesFalseCount, 1);
    assertEqual(control.totalInvocations(), 1);
    assertEqual(int(op.loggingValues), int(false));
    assertEqual(int(op.heating), int(false));
    control.resetCounters();

    // transition (event REC_ON) => state STANDBY
    automaton.transition(EVENT_REC_ON);

    
  }
  
#endif

