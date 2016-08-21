#include "bc_setup.h"
#ifdef UT_STATE
  #line 4 "ut_state.cpp"
  #include <ArduinoUnitX.h>
  #include "state.h"
  #include "ut_state.h"

  //#define DEBUG_UT_STATE
  
  /*
   * Class MockControlActions
   */
  uint16_t MockControlActions::totalInvocations() {
    return 
      heatTrueCount +
      heatFalseCount +
      setConfigParamCount +
      requestHelpCount +
      requestLogCount +
      requestConfigCount +
      requestStatCount;
  }
   
  void MockControlActions::resetCounters() {
    heatTrueCount = 0;
    heatFalseCount = 0;
    setConfigParamCount = 0;
    requestHelpCount = 0;
    requestLogCount = 0;
    requestConfigCount = 0;
    requestStatCount = 0;
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

  void MockControlActions::setConfigParam() {
    ControlActions::setConfigParam();
    setConfigParamCount++;
  }
  
  void MockControlActions::requestHelp() {
    ControlActions::requestHelp();
    requestHelpCount++;
  }
  
  void MockControlActions::requestLog() {
    ControlActions::requestLog();
    requestLogCount++;
  }
  
  void MockControlActions::requestConfig() {
    ControlActions::requestConfig();
    requestConfigCount++;
  }
  
  void MockControlActions::requestStat() {
    ControlActions::requestStat();
    requestStatCount++;
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
  
  Timestamp MockLog::logValues(Temperature, Temperature, Flags) {
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
    
    UserCommand cmd;
    memset(cmd.args, 0, CMD_ARG_BUF_SIZE);
    
    OperationalParams op;
    op.command = &cmd;
    
    ExecutionContext context;
    MockControlActions control = MockControlActions(&context);

    context.log = &logger;
    context.config = &config;
    context.op = &op;
    context.control = &control;
    
    BoilerStateAutomaton automaton;
    automaton.init(&context);

    assertEqual(automaton.state()->id(), STATE_INIT);
    assertEqual(int16_t(automaton.userCommands()), int16_t(CMD_NONE));
    assertEqual(control.totalInvocations(), 0);

    // water sensor = SENSOR_INITIALISING => evaluate => no event
    assertEqual(int16_t(context.op->water.sensorStatus), int16_t(SENSOR_INITIALISING));
    EventCandidates cand = automaton.evaluate();
    assertEqual(int16_t(cand), int16_t(EVENT_NONE));
    assertEqual(automaton.state()->id(), STATE_INIT);
    assertEqual(control.totalInvocations(), 0);

    // transition (INVALID event) => stay in state INIT
    automaton.transition(EVENT_SET_CONFIG); 
    assertEqual(automaton.state()->id(), STATE_INIT);
    assertEqual(control.totalInvocations(), 0);
    // invalid event logging
    assertEqual(logger.logMessageCount, 1);
    assertEqual(logger.totalInvocations(), 1);
    logger.resetCounters();

    // transition again (INVALID event) => must NOT LOG again
    automaton.transition(EVENT_SET_CONFIG); 
    assertEqual(logger.totalInvocations(), 0);

    //
    // set water sensor OK => evaluate => event READY
    //
    op.water.sensorStatus = SENSOR_OK;
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
    // user command SET_CONFIG in state IDLE
    //
    assertEqual(int16_t(automaton.userCommands()), int16_t(CMD_HELP | CMD_GET_CONFIG |  CMD_GET_LOG | CMD_GET_STAT | CMD_SET_CONFIG | CMD_RESET_CONFIG | CMD_REC_ON));
    op.command->command = CMD_SET_CONFIG;
    cand = automaton.evaluate();
    assertEqual(int16_t(cand), int16_t(EVENT_SET_CONFIG));
    op.command->command = CMD_NONE;
    
    // transition state IDLE => event SET_CONFIG => stay in state IDLE
    automaton.transition(EVENT_SET_CONFIG); 
    assertEqual(automaton.state()->id(), STATE_IDLE);
    assertEqual(control.setConfigParamCount, 1);
    assertEqual(control.totalInvocations(), 1);
    control.resetCounters();

    //
    // user command REC ON in state IDLE
    //
    op.command->command = CMD_REC_ON;
    cand = automaton.evaluate();
    assertEqual(int16_t(cand), int16_t(EVENT_REC_ON));
    op.command->command = CMD_NONE;
    
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
    assertEqual(int16_t(automaton.userCommands()), int16_t(CMD_HELP | CMD_GET_CONFIG |  CMD_GET_LOG | CMD_GET_STAT | CMD_REC_OFF | CMD_HEAT_ON));
    op.command->command = CMD_REC_OFF;
    cand = automaton.evaluate();
    assertEqual(int16_t(cand), int16_t(EVENT_REC_OFF));
    op.command->command = CMD_NONE;
    
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
    op.command->command = CMD_HEAT_ON;
    cand = automaton.evaluate();
    assertEqual(int16_t(cand), int16_t(EVENT_HEAT_ON));
    op.command->command = CMD_NONE;
    
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
    op.command->command = CMD_HEAT_OFF;
    cand = automaton.evaluate();
    assertEqual(int16_t(cand), int16_t(EVENT_HEAT_OFF));
    op.command->command = CMD_NONE;
    
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
    UserCommand cmd;
    memset(cmd.args, 0, CMD_ARG_BUF_SIZE);
    op.command = &cmd;
    
    ExecutionContext context;
    
    MockControlActions control = MockControlActions(&context);

    context.log = &logger;
    context.config = &config;
    context.op = &op;
    context.control = &control;

    BoilerStateAutomaton automaton;
    automaton.init(&context);

    assertEqual(automaton.state()->id(), STATE_INIT);
    assertEqual(int16_t(automaton.userCommands()), int16_t(CMD_NONE));
    assertEqual(control.totalInvocations(), 0);

    //
    // set water sensor NOK => evaluate => event SENSORS_NOK
    //
    op.water.sensorStatus = SENSOR_NOK;
    EventCandidates cand = automaton.evaluate();
    assertEqual(int16_t(cand), int16_t(EVENT_SENSORS_NOK));
    
    // transition state INIT => event EVENT_SENSORS_NOK => state STATE_SENSORS_NOK
    automaton.transition(EVENT_SENSORS_NOK);
    assertEqual(automaton.state()->id(), STATE_SENSORS_NOK);
    assertEqual(control.totalInvocations(), 0);
    // state logging
    assertEqual(logger.logStateCount, 1);
    assertEqual(logger.totalInvocations(), 1);
    logger.resetCounters();
  }
  
#endif

