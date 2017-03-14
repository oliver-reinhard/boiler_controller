#include "bc_setup.h"
#ifdef UT_STATE
  #line 4 "ut_state.cpp"
  #include <ArduinoUnitX.h>
  #include "state.h"
  #include "ut_state.h"

  //#define DEBUG_UT_STATE

  static const UserCommands ALL_CMD_INFO = CMD_INFO_HELP | CMD_INFO_CONFIG |  CMD_INFO_LOG | CMD_INFO_STAT;
  static const UserCommands ALL_CMD_CONFIG_MODIFY = CMD_CONFIG_SET_VALUE | CMD_CONFIG_SWAP_IDS | CMD_CONFIG_CLEAR_IDS | CMD_CONFIG_ACK_IDS | CMD_CONFIG_RESET_ALL;
  
  /*
   * Class MockControlActions
   */
  uint16_t MockControlActions::totalInvocations() {
    return 
      heatTrueCount +
      heatFalseCount +
      modifyConfigCount;
  }
   
  void MockControlActions::resetCounters() {
    heatTrueCount = 0;
    heatFalseCount = 0;
    modifyConfigCount = 0;
  }


  void MockControlActions::heat(boolean on) {
    #ifdef DEBUG_UT_STATE
      Serial.println(F("DEBUG_UT_STATE: heat (on/off)"));
    #endif
    context->op->heating = on;
    if (on) {
      heatTrueCount++;
    } else {
      heatFalseCount++;
    }
  }

  void MockControlActions::modifyConfig() {
    ControlActions::modifyConfig();
    modifyConfigCount++;
  }

  /*
   * Class MockLog
   */
  uint16_t MockLog::totalInvocations() {
    return 
      logValuesCount +
      logStateCount +
      logMessageCount;
  }
   
  void MockLog::resetCounters() {
    logValuesCount = 0;
    logStateCount = 0;
    logMessageCount = 0;
  }
  
  
  Timestamp MockLog::logMessage(MessageID, int16_t, int16_t) {    
    #ifdef DEBUG_UT_STATE
      Serial.println(F("DEBUG_UT_STATE: logMessage(...)"));
    #endif
    logMessageCount++;
    return logTime.timestamp();
  }
  
  Timestamp MockLog::logValues(CF_Temperature, CF_Temperature, Flags) {
    #ifdef DEBUG_UT_STATE
      Serial.println(F("DEBUG_UT_STATE: logValues(...)"));
    #endif
    logValuesCount++;
    return logTime.timestamp();
  }
  
  Timestamp MockLog::logState(StateID, StateID, EventID) {
    #ifdef DEBUG_UT_STATE
      Serial.println(F("DEBUG_UT_STATE: logState(...)"));
    #endif
    logStateCount++;
    return logTime.timestamp();
  }
  
  /*
   * Tests
   */
  test(state_automaton_sensors_ok) {
    MockConfig config = MockConfig();
    MockLog logger = MockLog();
    boolean updated;
    config.initParams(updated);
    #ifdef DEBUG_UT_STATE
      printConfig(config);
    #endif
    assertEqual(config.heaterCutOutWaterTemp, DEFAULT_HEATER_CUT_OUT_WATER_TEMP); 
    
    OperationalParams op;
    op.request.clear();
    
    ExecutionContext context;
    UserFeedback feedback;
    MockControlActions control = MockControlActions(&context, &feedback);

    context.log = &logger;
    context.config = &config;
    context.op = &op;
    context.control = &control;
    
    BoilerStateAutomaton automaton;
    automaton.init(&context);

    assertEqual(automaton.state()->id(), STATE_INIT);
    assertEqual(int16_t(automaton.acceptedUserCommands()), int16_t(CMD_NONE));
    assertEqual(control.totalInvocations(), 0);

    // water sensor = DS18B20_SENSOR_INITIALISING => evaluate => no event
    assertEqual(int16_t(context.op->water.sensorStatus), int16_t(DS18B20_SENSOR_INITIALISING));
    EventCandidates cand = automaton.evaluate();
    assertEqual(int16_t(cand), int16_t(EVENT_NONE));
    assertEqual(automaton.state()->id(), STATE_INIT);
    assertEqual(control.totalInvocations(), 0);

    // transition (INVALID event) => stay in state INIT
    automaton.transition(EVENT_CONFIG_MODIFY); 
    assertEqual(automaton.state()->id(), STATE_INIT);
    assertEqual(control.totalInvocations(), 0);
    // invalid event logging
    assertEqual(logger.logMessageCount, 1);
    assertEqual(logger.totalInvocations(), 1);
    logger.resetCounters();

    // transition again (INVALID event) => must NOT LOG again
    automaton.transition(EVENT_CONFIG_MODIFY); 
    assertEqual(logger.totalInvocations(), 0);

    //
    // set water sensor OK => evaluate => event READY
    //
    op.water.sensorStatus = DS18B20_SENSOR_OK;
    op.water.currentTemp = 2300;
    cand = automaton.evaluate();
    assertEqual(int16_t(cand), int16_t(EVENT_READY));
    assertEqual(automaton.state()->id(), STATE_INIT);
    assertEqual(control.totalInvocations(), 0);
    assertEqual(logger.totalInvocations(), 0);
    

    // transition state INIT => event READY => state IDLE
    automaton.transition(EVENT_READY);
    assertEqual(automaton.state()->id(), STATE_IDLE);
    assertEqual(control.totalInvocations(), 0);
    assertEqual(int16_t(op.loggingValues), int16_t(false));
    assertEqual(int16_t(op.heating), int16_t(false));
    // state logging
    assertEqual(logger.logStateCount, 1);
    assertEqual(logger.totalInvocations(), 1);
    logger.resetCounters();

    //
    // user command CONFIG_MODIFY in state IDLE
    //
    assertEqual(int16_t(automaton.acceptedUserCommands()), int16_t(ALL_CMD_INFO | ALL_CMD_CONFIG_MODIFY | CMD_CONFIG_RESET_ALL | CMD_REC_ON));
    op.request.command = CMD_CONFIG_SET_VALUE;
    op.request.event = EVENT_CONFIG_MODIFY;
    cand = automaton.evaluate();
    assertEqual(int16_t(cand), int16_t(EVENT_CONFIG_MODIFY));
    op.request.command = CMD_NONE;
    
    // transition state IDLE => event CONFIG_MODIFY => stay in state IDLE
    automaton.transition(EVENT_CONFIG_MODIFY); 
    assertEqual(automaton.state()->id(), STATE_IDLE);
    assertEqual(control.modifyConfigCount, 1);
    assertEqual(control.totalInvocations(), 1);
    control.resetCounters();

    //
    // user command REC ON in state IDLE
    //
    op.request.command = CMD_REC_ON;
    op.request.event = EVENT_REC_ON;
    cand = automaton.evaluate();
    assertEqual(int16_t(cand), int16_t(EVENT_REC_ON));
    op.request.command = CMD_NONE;
    
    // transition state IDLE => event REC_ON => state STANDBY
    assertEqual(int16_t(op.loggingValues), int16_t(false));
    automaton.transition(EVENT_REC_ON); 
    assertEqual(automaton.state()->id(), STATE_STANDBY);
    assertEqual(int16_t(op.loggingValues), int16_t(true));
    assertEqual(int16_t(op.heating), int16_t(false));
    control.resetCounters();

    //
    // user command REC OFF in state STANDBY
    //
    assertEqual(int16_t(automaton.acceptedUserCommands()), int16_t(ALL_CMD_INFO | CMD_REC_OFF | CMD_HEAT_ON));
    op.request.command = CMD_REC_OFF;
    op.request.event = EVENT_REC_OFF;
    cand = automaton.evaluate();
    assertEqual(int16_t(cand), int16_t(EVENT_REC_OFF));
    op.request.command = CMD_NONE;
    
    // transition state STANDBY => event REC_OFF => state IDLE
    automaton.transition(EVENT_REC_OFF); 
    assertEqual(automaton.state()->id(), STATE_IDLE);
    assertEqual(int16_t(op.loggingValues), int16_t(false));
    assertEqual(int16_t(op.heating), int16_t(false));
    control.resetCounters();

    // transition state IDLE => event REC_ON => state STANDBY
    automaton.transition(EVENT_REC_ON);
    control.resetCounters();
    
    //
    // user command HEAT ON in state STANDBY
    //
    op.request.command = CMD_HEAT_ON;
    op.request.event = EVENT_HEAT_ON;
    cand = automaton.evaluate();
    assertEqual(int16_t(cand), int16_t(EVENT_HEAT_ON));
    op.request.command = CMD_NONE;
    
    // transition state STANDBY => event HEAT_ON => state HEATING
    automaton.transition(EVENT_HEAT_ON); 
    assertEqual(automaton.state()->id(), STATE_HEATING);
    assertEqual(control.heatTrueCount, 1);
    assertEqual(control.totalInvocations(), 1);
    assertEqual(int16_t(op.loggingValues), int16_t(true));
    assertEqual(int16_t(op.heating), int16_t(true));
    control.resetCounters();

    // evaluate => no event
    assertEqual(config.heaterCutOutWaterTemp, DEFAULT_HEATER_CUT_OUT_WATER_TEMP); 
    cand = automaton.evaluate();
    assertEqual(int16_t(cand), int16_t(EVENT_NONE));

    //
    // set water-sensor temp to overheated
    //
    op.water.currentTemp = DEFAULT_HEATER_CUT_OUT_WATER_TEMP + 1;
    cand = automaton.evaluate();
    assertEqual(int16_t(cand), int16_t(EVENT_TEMP_OVER));

    // transition state HEATING => event EVENT_TEMP_OVER => state OVERHEATED
    automaton.transition(EVENT_TEMP_OVER); 
    assertEqual(automaton.state()->id(), STATE_OVERHEATED);
    assertEqual(control.heatFalseCount, 1);
    assertEqual(control.totalInvocations(), 1);
    assertEqual(int16_t(op.loggingValues), int16_t(true));
    assertEqual(int16_t(op.heating), int16_t(false));
    control.resetCounters();
    
    // evaluate => no event
    cand = automaton.evaluate();
    assertEqual(int16_t(cand), int16_t(EVENT_NONE));

    //
    // set water-sensor temp to ok
    //
    op.water.currentTemp = DEFAULT_HEATER_BACK_OK_WATER_TEMP -1;
    assertEqual(config.heaterBackOkWaterTemp, DEFAULT_HEATER_BACK_OK_WATER_TEMP); 
    cand = automaton.evaluate();
    assertEqual(int16_t(cand), int16_t(EVENT_TEMP_OK));
    op.water.currentTemp = 4500;

    // transition state OVERHEATED => event EVENT_TEMP_OK => state HEATING
    automaton.transition(EVENT_TEMP_OK); 
    assertEqual(automaton.state()->id(), STATE_HEATING);
    assertEqual(control.heatTrueCount, 1);
    assertEqual(control.totalInvocations(), 1);
    assertEqual(int16_t(op.loggingValues), int16_t(true));
    assertEqual(int16_t(op.heating), int16_t(true));
    control.resetCounters();
    
    //
    // user command HEAT OFF in state HEATING
    //
    op.request.command = CMD_HEAT_OFF;
    op.request.event = EVENT_HEAT_OFF;
    cand = automaton.evaluate();
    assertEqual(int16_t(cand), int16_t(EVENT_HEAT_OFF));
    op.request.command = CMD_NONE;
    op.request.event = EVENT_NONE;
    
    // transition state HEATING => event HEAT_OFF => state STANDBY
    automaton.transition(EVENT_HEAT_OFF); 
    assertEqual(automaton.state()->id(), STATE_STANDBY);
    assertEqual(control.heatFalseCount, 1);
    assertEqual(control.totalInvocations(), 1);
    assertEqual(int16_t(op.loggingValues), int16_t(true));
    assertEqual(int16_t(op.heating), int16_t(false));
    control.resetCounters();
  }

  
  test(state_automaton_sensors_nok) {
    MockLog logger = MockLog();
    MockConfig config = MockConfig();
    boolean updated;
    config.initParams(updated);
    
    OperationalParams op;
    op.request.clear();
    
    ExecutionContext context;
    UserFeedback feedback;
    MockControlActions control = MockControlActions(&context, &feedback);

    context.log = &logger;
    context.config = &config;
    context.op = &op;
    context.control = &control;

    BoilerStateAutomaton automaton;
    automaton.init(&context);

    assertEqual(automaton.state()->id(), STATE_INIT);
    assertEqual(int16_t(automaton.acceptedUserCommands()), int16_t(CMD_NONE));
    assertEqual(control.totalInvocations(), 0);

    //
    // set water sensor NOK => evaluate => event SENSORS_NOK
    //
    op.water.sensorStatus = DS18B20_SENSOR_NOK;
    EventCandidates cand = automaton.evaluate();
    assertEqual(int16_t(cand), int16_t(EVENT_SENSORS_NOK));
    
    // transition state INIT => event EVENT_SENSORS_NOK => state STATE_SENSORS_NOK
    automaton.transition(EVENT_SENSORS_NOK);
    assertEqual(automaton.state()->id(), STATE_SENSORS_NOK);
    assertEqual(int16_t(automaton.acceptedUserCommands()), int16_t(ALL_CMD_INFO | ALL_CMD_CONFIG_MODIFY | CMD_CONFIG_RESET_ALL));
    assertEqual(control.totalInvocations(), 0);
    // state logging
    assertEqual(logger.logStateCount, 1);
    assertEqual(logger.totalInvocations(), 1);
    logger.resetCounters();
  }
  
#endif

