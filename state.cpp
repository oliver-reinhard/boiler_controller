#include "state.h"

// #define DEBUG_STATE

/*
 * ABSTRACT STATE
 */
void AbstractState::init(ExecutionContext *context) {
   this->context = context;
}
      
UserCommands AbstractState::userCommands() {
  // default implementation: delegate to containing state (if any):
  if(containingState != NULL) { 
    return containingState->userCommands();
  } else {
    return CMD_NONE;
  }
}

EventCandidates AbstractState::eval() {
  // default implementation: delegate to containing state (if any):
  if(containingState != NULL) { 
    return containingState->eval();
  } else {
    return EVENT_NONE;
  }
}

StateEnum AbstractState::transAction(EventEnum) {
  // default implementation: empty
  return STATE_UNDEFINED;
}

void AbstractState::entryAction() {
  // default implementation: empty
}

void AbstractState::exitAction(){
  // default implementation: empty
}


/*
 * SIMPLE STATE
 */
StateEnum AbstractSimpleState::enter() {
  entryAction();
  return id();
}
        
StateEnum AbstractSimpleState::trans(EventEnum event) {
  StateEnum next = transAction(event);
  if (next == STATE_UNDEFINED && containingState != NULL) {
    next = containingState->trans(event);
  }
  if (next != STATE_SAME && next != STATE_UNDEFINED) {
    exit(event, next);
  }
  return next;
}

void AbstractSimpleState::exit(EventEnum event, StateEnum next) {
  exitAction();
  if(containingState != NULL) { 
    containingState->exit(event, next);
  }
}

/*
 * COMPOSITE STATE
 */
void AbstractCompositeState::init(ExecutionContext *context, AbstractState **substates, uint16_t numSubstates) {
  AbstractState::init(context);
  this->substates = substates;
  this->numSubstates = numSubstates;
  // set containingStates of the subtates to "this":
  for(uint16_t i=0; i< this->numSubstates; i++) {
    this->substates[i]->containingState = this;
  }
}

AbstractState *AbstractCompositeState::initialSubstate() {
  return substates[0];
}
      
StateEnum AbstractCompositeState::enter() {
  entryAction();
  return initialSubstate()->enter();
}

StateEnum AbstractCompositeState::trans(EventEnum event) {
  StateEnum next = transAction(event);
  if (next == STATE_UNDEFINED && containingState != NULL) {
    next = containingState->trans(event);
  }
  return next;
  // Do not invoke the exit action here: exits are performed "up" the containment hiearchy.
}

void AbstractCompositeState::exit(EventEnum event, StateEnum next) {
  // if the transition occurs within the substates of this containing state, then we will not exit this containing state
  // and neither its containing states:
  for(uint16_t i=0; i< numSubstates; i++) {
    if (substates[i]->id() == next) {
      return;
    }
  }
  exitAction();
  if(containingState != NULL) { 
    containingState->exit(event, next);
  }
}

/*
 * INIT
 */

// No user commands available.

EventCandidates Init::eval() {
  EventCandidates result = AbstractState::eval();
  if (context->op->water.sensorStatus == SENSOR_INITIALISING) {
    result |= EVENT_NONE;  // = wait
  } else if (context->op->water.sensorStatus == SENSOR_OK) {
    result |= EVENT_READY;
  } else  {
    result |= EVENT_SENSORS_NOK;
  } 
  return result;
}

StateEnum Init::transAction(EventEnum event) {
  if (event == EVENT_READY) {
    return STATE_READY;
  } else if (event == EVENT_SENSORS_NOK) {
    return STATE_SENSORS_NOK;
  }
  return AbstractState::transAction(event);
}

/*
 * SENSORS NOK
 */

UserCommands SensorsNOK::userCommands() {
  return AbstractState::userCommands() | CMD_HELP | CMD_SET_CONFIG | CMD_RESET_CONFIG | CMD_GET_CONFIG |  CMD_GET_LOG | CMD_GET_STAT;
}

EventCandidates SensorsNOK::eval() {
  EventCandidates result = AbstractState::eval();
  if (context->op->command->command == CMD_HELP) {
    result |= EVENT_HELP;
  } else if (context->op->command->command == CMD_SET_CONFIG) {
    result |= EVENT_SET_CONFIG;
  } else if (context->op->command->command == CMD_RESET_CONFIG) {
    result |= EVENT_RESET_CONFIG;
  } else if (context->op->command->command == CMD_GET_CONFIG) {
    result |= EVENT_GET_CONFIG;
  } else if (context->op->command->command == CMD_GET_LOG) {
    result |= EVENT_GET_LOG;
  } else if (context->op->command->command == CMD_GET_STAT) {
    result |= EVENT_GET_STAT;
  }
  return result;
}

StateEnum SensorsNOK::transAction(EventEnum event) {
  if (event == EVENT_HELP) {
    context->control->requestHelp();
    return STATE_SAME;
    
  } else if (event == EVENT_SET_CONFIG) {
    context->control->setConfigParam();
    return STATE_SAME;
    
  } else if (event == EVENT_GET_CONFIG) {
    context->control->requestConfig();
    return STATE_SAME;
    
  } else if (event == EVENT_GET_LOG) {
    context->control->requestLog();
    return STATE_SAME;
    
  } else if (event == EVENT_GET_STAT) {
    context->control->requestStat();
    return STATE_SAME;
  }
  return AbstractState::transAction(event);
}

/*
 * READY
 */

UserCommands Ready::userCommands() {
  return AbstractState::userCommands() | CMD_HELP | CMD_GET_CONFIG |  CMD_GET_LOG | CMD_GET_STAT;
}

EventCandidates Ready::eval() {
  EventCandidates result = AbstractState::eval();
  if (context->op->water.sensorStatus == SENSOR_NOK) {
    result |= EVENT_SENSORS_NOK;
  } 
  if (context->op->command->command == CMD_HELP) {
    result |= EVENT_HELP;
  } else if (context->op->command->command == CMD_GET_CONFIG) {
    result |= EVENT_GET_CONFIG;
  } else if (context->op->command->command == CMD_GET_LOG) {
    result |= EVENT_GET_LOG;
  } else if (context->op->command->command == CMD_GET_STAT) {
    result |= EVENT_GET_STAT;
  }
  return result;
}

StateEnum Ready::transAction(EventEnum event) {
  if (event == EVENT_SENSORS_NOK) {
    return STATE_SENSORS_NOK;
    
  } else if (event == EVENT_HELP) {
    context->control->requestHelp();
    return STATE_SAME;
    
  } else if (event == EVENT_GET_CONFIG) {
    context->control->requestConfig();
    return STATE_SAME;
    
  } else if (event == EVENT_GET_LOG) {
    context->control->requestLog();
    return STATE_SAME;
    
  } else if (event == EVENT_GET_STAT) {
    context->control->requestStat();
    return STATE_SAME;
  }
  return AbstractState::transAction(event);
}

/*
 * IDLE
 */

UserCommands Idle::userCommands() {
  return AbstractState::userCommands() | CMD_SET_CONFIG | CMD_RESET_CONFIG | CMD_REC_ON;
}

EventCandidates Idle::eval() {
  EventCandidates result = AbstractState::eval();
  if (context->op->command->command == CMD_SET_CONFIG) {
    result |= EVENT_SET_CONFIG;
  } else if (context->op->command->command == CMD_RESET_CONFIG) {
    result |= EVENT_RESET_CONFIG;
  } else  if (context->op->command->command == CMD_REC_ON) {
    result |= EVENT_REC_ON;
  }
  return result;
}

StateEnum Idle::transAction(EventEnum event) {
  if (event == EVENT_SET_CONFIG) {
    context->control->setConfigParam();
    return STATE_SAME;
    
  } else if (event == EVENT_RESET_CONFIG) {
    context->config->reset();
    context->control->setupSensors();
    return STATE_SENSORS_NOK;
    
  } else if (event == EVENT_REC_ON) {
    return STATE_RECORDING;
  }
  return AbstractState::transAction(event);
}

/*
 * RECORDING
 */

UserCommands Recording::userCommands() {
  return AbstractState::userCommands() | CMD_REC_OFF;
}


EventCandidates Recording::eval() {
  EventCandidates result = AbstractState::eval();
  if (context->op->command->command == CMD_REC_OFF) {
    result |= EVENT_REC_OFF;
  }
  return result;
}

StateEnum Recording::transAction(EventEnum event) {
  if (event == EVENT_REC_OFF) {
    return STATE_IDLE;
  }
  return AbstractState::transAction(event);
}

void Recording::entryAction() {
  context->op->loggingValues = true;
}

void Recording::exitAction(){
  context->op->loggingValues = false;
}

/*
 * STANDBY
 */

UserCommands Standby::userCommands() {
  return AbstractState::userCommands() | CMD_HEAT_ON;
}

EventCandidates Standby::eval() {
  EventCandidates result = AbstractState::eval();
  if (context->op->command->command == CMD_HEAT_ON) {
    result |= EVENT_HEAT_ON;
  }
  return result;
}

StateEnum Standby::transAction(EventEnum event) {
  if (event == EVENT_HEAT_ON) {
    return STATE_HEATING;
  }
  return AbstractState::transAction(event);
}

/*
 * HEATING
 */

UserCommands Heating::userCommands() {
  return AbstractState::userCommands() | CMD_HEAT_OFF;
}

EventCandidates Heating::eval() {
  EventCandidates result = AbstractState::eval();
  if (context->op->command->command == CMD_HEAT_OFF) {
    result |= EVENT_HEAT_OFF;
  }
  if (context->op->water.currentTemp >= context->config->heaterCutOutWaterTemp) {
    result |= EVENT_TEMP_OVER;
  }
  return result;
}

StateEnum Heating::transAction(EventEnum event) {
  if (event == EVENT_HEAT_OFF) {
    return STATE_STANDBY;
  } else if (event == EVENT_TEMP_OVER) {
    return STATE_OVERHEATED;
  }
  return AbstractState::transAction(event);
}

void Heating::entryAction() {
  context->control->heat(true);
  context->op->heatingStartMillis = millis();
}

void Heating::exitAction(){
  context->control->heat(false);
  context->op->heatingAccumulatedMillis += millis() - context->op->heatingStartMillis;
  context->op->heatingStartMillis = 0L;
}

/*
 * OVERHEATED
 */

UserCommands Overheated::userCommands() {
  return AbstractState::userCommands() | CMD_HEAT_RESET;
}

EventCandidates Overheated::eval() {
  EventCandidates result = AbstractState::eval();
  if (context->op->command->command == CMD_HEAT_RESET) {
    result |= EVENT_HEAT_RESET;
  }
  if (context->op->water.currentTemp <= context->config->heaterBackOkWaterTemp) {
    result |= EVENT_TEMP_OK;
  }
  return result;
}

StateEnum Overheated::transAction(EventEnum event) {
  if (event == EVENT_HEAT_RESET) {
    return STATE_STANDBY;
    
  } else if (event == EVENT_TEMP_OK) {
    return STATE_HEATING;
  }
  return AbstractState::transAction(event);
}

/*
 * STATE AUTOMATON
 */
void BoilerStateAutomaton::init(ExecutionContext *context) {
  this->context = context;
  
  INIT.init(context);
  IDLE.init(context);
  SENSORS_NOK.init(context);
  STANDBY.init(context);
  HEATING.init(context);
  OVERHEATED.init(context);
  RECORDING.init(context, RECORDING_SUBSTATES, 3);
  READY.init(context, READY_SUBSTATES, 2);

  ALL_STATES[STATE_INIT] = &INIT;
  ALL_STATES[STATE_SENSORS_NOK] = &SENSORS_NOK;
  ALL_STATES[STATE_READY] = &READY;
  ALL_STATES[STATE_IDLE] = &IDLE;
  ALL_STATES[STATE_RECORDING] = &RECORDING;
  ALL_STATES[STATE_STANDBY] = &STANDBY;
  ALL_STATES[STATE_HEATING] = &HEATING;
  ALL_STATES[STATE_OVERHEATED] = &OVERHEATED;
  
  currentState = ALL_STATES[STATE_INIT];
 }
 
AbstractState *BoilerStateAutomaton::state() {
  return currentState;
}
  
UserCommands BoilerStateAutomaton::userCommands() {
  return currentState->userCommands();
}
  
EventCandidates BoilerStateAutomaton::evaluate() {
  EventCandidates cand = currentState->eval();
  #ifdef DEBUG_STATE
    Serial.print(F("DEBUG_STATE: eval in state "));
    Serial.print(currentState->id());
    Serial.print(F(": candidates = "));
    Serial.println(cand, HEX);
  #endif
  return cand;
}

void BoilerStateAutomaton::transition(EventEnum event) {
  #ifdef DEBUG_STATE
    Serial.print(F("DEBUG_STATE: State "));
    Serial.print(currentState->id());
    Serial.print(F(": process event 0x"));
    Serial.println(event, HEX);
  #endif
  StateEnum oldState = currentState->id();
  StateEnum newState = currentState->trans(event);
  if (newState == STATE_UNDEFINED) {
    if ((event & currentState->illegalTransitionLogged) == 0) { // this illegal event has not been logged at this state
      #ifdef DEBUG_STATE
        Serial.print(F("DEBUG_STATE: State "));
        Serial.print(currentState->id());
        Serial.print(F(": log invalid event 0x"));
        Serial.println(event, HEX);
      #endif

      context->log->logMessage(MSG_ILLEGAL_TRANS, currentState->id(), event);
      currentState->illegalTransitionLogged |= event;
    }
    
  } else if (newState != STATE_SAME && oldState != newState) { // we are in a different new state!
    currentState = getState(newState);
    
    // Enter the new state (which can be a composite state but will always end up in a simple state):
    newState = currentState->enter();
    currentState = getState(newState);
    context->op->currentStateStartMillis = millis();
    
    context->log->logState(oldState, newState, event);
  }
  #ifdef DEBUG_STATE
    Serial.print(("DEBUG_STATE: New state: "));
    Serial.println(currentState->id());
  #endif
}

AbstractState *BoilerStateAutomaton::getState(StateEnum id) {
  for(uint16_t i=0; i<NUM_STATES; i++) {
    if (ALL_STATES[i]->id() == id) {
      return ALL_STATES[i];
    }
  }
  context->log->S_O_S(MSG_UNKNOWN_STATE, id, 0);  // function NEVER RETURNS
  abort();
}

