#include "unit_test.h"
#ifdef TEST_STATE
  #line 4 "test_state.cpp"
  #include <ArduinoUnit.h>
  #include "storage.h"
  #include "state.h"
  #include "test_state.h"

  //#define DEBUG_TEST_STATE
  
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
      
  void MockControlActions::readSensors(ControlContext *context) {
    if (context == NULL) { } // prevent warning "unused parameter ..."
  }
  
  void MockControlActions::readUserCommands(ControlContext *context) {
    if (context == NULL) { } // prevent warning "unused parameter ..."
  }

  void MockControlActions::heat(boolean on, ControlContext *context) {
    #ifdef DEBUG_TEST_STATE
      Serial.println("DEBUG_TEST_STATE: heat(on/off)");
    #endif
    context->op->heating = on;
    if (on) {
      heatTrueCount++;
    } else {
      heatFalseCount++;
    }
  }

  void MockControlActions::logValues(boolean on, ControlContext *context) {
    #ifdef DEBUG_TEST_STATE
      Serial.println("DEBUG_TEST_STATE: logValues(on/off)");
    #endif
    context->op->loggingValues = on;
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
   * Class MockStorage
   */
  unsigned short MockStorage::totalInvocations() {
    return 
      logValuesCount +
      logStateCount +
      logMessageCount;
  }
   
  void MockStorage::resetCounters() {
    logValuesCount = 0;
    logStateCount = 0;
    logMessageCount = 0;
  }

  Version MockStorage::version() {
    return EEPROM_LAYOUT_VERSION;
  }
  
  void MockStorage::clearConfigParams() {
    // none
  }
  
  void MockStorage::getConfigParams(ConfigParams *configParams) {
    if (configParams == NULL) { } // prevent warning "unused parameter ..."
  }
  
  void MockStorage::updateConfigParams(const ConfigParams *configParams) {
    if (configParams == NULL) { } // prevent warning "unused parameter ..."
  }
  
  void MockStorage::readConfigParams(ConfigParams *configParams) {
    if (configParams == NULL) { } // prevent warning "unused parameter ..."
  }
  
  void MockStorage::resetLog() {
    // none
  }
  
  void MockStorage::initLog() {
    // none
  }
  
  Timestamp MockStorage::logValues(Temperature water, Temperature ambient, Flags flags) {
    #ifdef DEBUG_TEST_STATE
      Serial.println("DEBUG_TEST_STATE: logValues(...)");
    #endif
    if (water == 0 || ambient == 0 || flags == 0) { } // prevent warning "unused parameter ..."
    logValuesCount++;
    return timestamp();
  }
  
  Timestamp MockStorage::logState(StateID previous, StateID current, EventID event) {
    #ifdef DEBUG_TEST_STATE
      Serial.println("DEBUG_TEST_STATE: logState(...)");
    #endif
    if (previous == 0 || current == 0 || event == 0) { } // prevent warning "unused parameter ..."
    logStateCount++;
    return timestamp();
  }
  
  Timestamp MockStorage::logMessage(MessageID id, short param1, short param2) {    
    #ifdef DEBUG_TEST_STATE
      Serial.println("DEBUG_TEST_STATE: logMessage(...)");
    #endif
    if (id == 0 || param1 == 0 || param2 == 0) { } // prevent warning "unused parameter ..."
    logMessageCount++;
    return timestamp();
  }
  

  /*
   * Tests
   */
  test(state_automaton) {
    MockStorage storage = MockStorage();
    ConfigParams config;
    boolean updated;
    storage.initConfigParams(&config, &updated);
    
    OperationalParams op;
    MockControlActions control;

    ExecutionContext context;
    context.storage = &storage;
    context.config = &config;
    context.op = &op;
    context.control = &control;

    BoilerStateAutomaton automaton = BoilerStateAutomaton(&context);

    assertEqual(automaton.state()->id(), STATE_INIT);
    assertEqual(int(automaton.userCommands()), int(CMD_NONE));
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
    // invalid event logging
    assertEqual(storage.logMessageCount, 1);
    assertEqual(storage.totalInvocations(), 1);
    storage.resetCounters();

    // transition again (INVALID event) => must NOT LOG again
    automaton.transition(EVENT_SET_CONFIG); 
    assertEqual(storage.totalInvocations(), 0);

    //
    // set water sensor OK => evaluate => event READY
    //
    op.water.sensorStatus = SENSOR_OK;
    op.water.currentTemp = 2300;
    cand = automaton.evaluate();
    assertEqual(int(cand), int(EVENT_READY));
    assertEqual(automaton.state()->id(), STATE_INIT);
    assertEqual(control.totalInvocations(), 0);
    assertEqual(storage.totalInvocations(), 0);
    

    // transition state INIT => event READY => state IDLE
    automaton.transition(EVENT_READY);
    assertEqual(automaton.state()->id(), STATE_IDLE);
    assertEqual(control.totalInvocations(), 0);
    assertEqual(int(op.loggingValues), int(false));
    assertEqual(int(op.heating), int(false));
    // state logging
    assertEqual(storage.logStateCount, 1);
    assertEqual(storage.totalInvocations(), 1);
    storage.resetCounters();

    //
    // user command SET_CONFIG in state IDLE
    //
    assertEqual(int(automaton.userCommands()), int(CMD_GET_CONFIG |  CMD_GET_LOG | CMD_GET_STAT | CMD_SET_CONFIG | CMD_REC_ON));
    op.userCommands = CMD_SET_CONFIG;
    cand = automaton.evaluate();
    assertEqual(int(cand), int(EVENT_SET_CONFIG));
    op.userCommands = CMD_NONE;
    
    // transition state IDLE => event SET_CONFIG => stay in state IDLE
    automaton.transition(EVENT_SET_CONFIG); 
    assertEqual(automaton.state()->id(), STATE_IDLE);
    assertEqual(control.setConfigParamCount, 1);
    assertEqual(control.totalInvocations(), 1);
    control.resetCounters();

    //
    // user command REC ON in state IDLE
    //
    op.userCommands = CMD_REC_ON;
    cand = automaton.evaluate();
    assertEqual(int(cand), int(EVENT_REC_ON));
    op.userCommands = CMD_NONE;
    
    // transition state IDLE => event REC_ON => state STANDBY
    automaton.transition(EVENT_REC_ON); 
    assertEqual(automaton.state()->id(), STATE_STANDBY);
    assertEqual(control.logValuesTrueCount, 1);
    assertEqual(control.totalInvocations(), 1);
    assertEqual(int(op.loggingValues), int(true));
    assertEqual(int(op.heating), int(false));
    control.resetCounters();

    //
    // user command REC OFF in state STANDBY
    //
    assertEqual(int(automaton.userCommands()), int(CMD_GET_CONFIG |  CMD_GET_LOG | CMD_GET_STAT | CMD_REC_OFF | CMD_HEAT_ON));
    op.userCommands = CMD_REC_OFF;
    cand = automaton.evaluate();
    assertEqual(int(cand), int(EVENT_REC_OFF));
    op.userCommands = CMD_NONE;
    
    // transition state STANDBY => event REC_OFF => state IDLE
    automaton.transition(EVENT_REC_OFF); 
    assertEqual(automaton.state()->id(), STATE_IDLE);
    assertEqual(control.logValuesFalseCount, 1);
    assertEqual(control.totalInvocations(), 1);
    assertEqual(int(op.loggingValues), int(false));
    assertEqual(int(op.heating), int(false));
    control.resetCounters();

    // transition state IDLE => event REC_ON => state STANDBY
    automaton.transition(EVENT_REC_ON);
    control.resetCounters();
    
    //
    // user command HEAT ON in state STANDBY
    //
    op.userCommands = CMD_HEAT_ON;
    cand = automaton.evaluate();
    assertEqual(int(cand), int(EVENT_HEAT_ON));
    op.userCommands = CMD_NONE;
    
    // transition state STANDBY => event HEAT_ON => state HEATING
    automaton.transition(EVENT_HEAT_ON); 
    assertEqual(automaton.state()->id(), STATE_HEATING);
    assertEqual(control.heatTrueCount, 1);
    assertEqual(control.totalInvocations(), 1);
    assertEqual(int(op.loggingValues), int(true));
    assertEqual(int(op.heating), int(true));
    control.resetCounters();

    // evaluate => no event
    cand = automaton.evaluate();
    assertEqual(int(cand), int(EVENT_NONE));

    //
    // set water-sensor temp to overheated
    //
    op.water.currentTemp = DEFAULT_HEATER_CUT_OUT_WATER_TEMP + 1;
    assertEqual(config.heaterCutOutWaterTemp, DEFAULT_HEATER_CUT_OUT_WATER_TEMP); 
    cand = automaton.evaluate();
    assertEqual(int(cand), int(EVENT_TEMP_OVER));

    // transition state HEATING => event EVENT_TEMP_OVER => state OVERHEATED
    automaton.transition(EVENT_TEMP_OVER); 
    assertEqual(automaton.state()->id(), STATE_OVERHEATED);
    assertEqual(control.heatFalseCount, 1);
    assertEqual(control.totalInvocations(), 1);
    assertEqual(int(op.loggingValues), int(true));
    assertEqual(int(op.heating), int(false));
    control.resetCounters();
    
    // evaluate => no event
    cand = automaton.evaluate();
    assertEqual(int(cand), int(EVENT_NONE));

    //
    // set water-sensor temp to ok
    //
    op.water.currentTemp = DEFAULT_HEATER_BACK_OK_WATER_TEMP -1;
    assertEqual(config.heaterBackOkWaterTemp, DEFAULT_HEATER_BACK_OK_WATER_TEMP); 
    cand = automaton.evaluate();
    assertEqual(int(cand), int(EVENT_TEMP_OK));
    op.water.currentTemp = 4500;

    // transition state OVERHEATED => event EVENT_TEMP_OK => state HEATING
    automaton.transition(EVENT_TEMP_OK); 
    assertEqual(automaton.state()->id(), STATE_HEATING);
    assertEqual(control.heatTrueCount, 1);
    assertEqual(control.totalInvocations(), 1);
    assertEqual(int(op.loggingValues), int(true));
    assertEqual(int(op.heating), int(true));
    control.resetCounters();
    
    //
    // user command HEAT OFF in state HEATING
    //
    op.userCommands = CMD_HEAT_OFF;
    cand = automaton.evaluate();
    assertEqual(int(cand), int(EVENT_HEAT_OFF));
    op.userCommands = CMD_NONE;
    
    // transition state HEATING => event HEAT_OFF => state STANDBY
    automaton.transition(EVENT_HEAT_OFF); 
    assertEqual(automaton.state()->id(), STATE_STANDBY);
    assertEqual(control.heatFalseCount, 1);
    assertEqual(control.totalInvocations(), 1);
    assertEqual(int(op.loggingValues), int(true));
    assertEqual(int(op.heating), int(false));
    control.resetCounters();
  }
  
#endif

