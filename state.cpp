#include "state.h"
#include "storage.h"
#include <assert.h>

//#define DEBUG_STATE

/*
 * ABSTRACT STATE
 */
UserCommands AbstractState::userCommands(ExecutionContext *context) {
  // default implementation: delegate to containing state (if any):
  if(containingState != NULL) { 
    return containingState->userCommands(context);
  } else {
    return CMD_NONE;
  }
}

EventCandidates AbstractState::eval(ExecutionContext *context) {
  // default implementation: delegate to containing state (if any):
  if(containingState != NULL) { 
    return containingState->eval(context);
  } else {
    return EVENT_NONE;
  }
}

StateEnum AbstractState::transAction(EventEnum event, ExecutionContext *context) {
  if (event == EVENT_NONE || context == NULL) { }  // prevent compiler warning "unused parameter"
  // default implementation: empty
  return STATE_UNDEFINED;
}

void AbstractState::entryAction(ExecutionContext *context) {
  if (context == NULL) { }  // prevent compiler warning "unused parameter"
  // default implementation: empty
}

void AbstractState::exitAction(ExecutionContext *context){
  if (context == NULL) { }  // prevent compiler warning "unused parameter"
  // default implementation: empty
}


/*
 * SIMPLE STATE
 */
StateEnum AbstractSimpleState::enter(ExecutionContext *context) {
  entryAction(context);
  return id();
}
        
StateEnum AbstractSimpleState::trans(EventEnum event, ExecutionContext *context) {
  StateEnum next = transAction(event, context);
  if (next == STATE_UNDEFINED && containingState != NULL) {
    next = containingState->trans(event, context);
  }
  if (next != STATE_SAME && next != STATE_UNDEFINED) {
    exit(event, next, context);
  }
  return next;
}

void AbstractSimpleState::exit(EventEnum event, StateEnum next, ExecutionContext *context) {
  exitAction(context);
  if(containingState != NULL) { 
    containingState->exit(event, next, context);
  }
}

/*
 * COMPOSITE STATE
 */
// Constructor
AbstractCompositeState::AbstractCompositeState(AbstractState **substates, unsigned short numSubstates) {
  this->substates = substates;
  this->numSubstates = numSubstates;
  // set containingStates of the subtates to "this":
  for(unsigned short i=0; i< this->numSubstates; i++) {
    this->substates[i]->containingState = this;
  }
}

AbstractState *AbstractCompositeState::initialSubstate() {
  return substates[0];
}
      
StateEnum AbstractCompositeState::enter(ExecutionContext *context) {
  entryAction(context);
  return initialSubstate()->enter(context);
}

StateEnum AbstractCompositeState::trans(EventEnum event, ExecutionContext *context) {
  StateEnum next = transAction(event, context);
  if (next == STATE_UNDEFINED && containingState != NULL) {
    next = containingState->trans(event, context);
  }
  return next;
  // Do not invoke the exit action here: exits are performed "up" the containment hiearchy.
}

void AbstractCompositeState::exit(EventEnum event, StateEnum next, ExecutionContext *context) {
  // if the transition occurs within the substates of this containing state, then we will not exit this containing state
  // and neither its containing states:
  for(unsigned short i=0; i< numSubstates; i++) {
    if (substates[i]->id() == next) {
      return;
    }
  }
  exitAction(context);
  if(containingState != NULL) { 
    containingState->exit(event, next, context);
  }
}

/*
 * INIT
 */
StateEnum Init::id() {
  return STATE_INIT;
}

// No user commands available.

EventCandidates Init::eval(ExecutionContext *context) {
  EventCandidates result = AbstractState::eval(context);
  if (context->op->water.sensorStatus == SENSOR_OK && context->op->ambient.sensorStatus != SENSOR_ID_UNDEFINED) {
    result |= EVENT_READY;
  } else if (context->op->water.sensorStatus == SENSOR_NOK 
    || context->op->water.sensorStatus == SENSOR_ID_UNDEFINED
    || context->op->ambient.sensorStatus == SENSOR_ID_UNDEFINED) {
    result |= EVENT_SENSORS_NOK;
  }
  return result;
}

StateEnum Init::transAction(EventEnum event, ExecutionContext *context) {
  if (event == EVENT_READY) {
    return STATE_READY;
  } else if (event == EVENT_SENSORS_NOK) {
    return STATE_SENSORS_NOK;
  }
  return AbstractState::transAction(event, context);
}

/*
 * SENSORS NOK
 */
StateEnum SensorsNOK::id() {
  return STATE_SENSORS_NOK;
}

// Terminal state: No user or events available commands available.

/*
 * READY
 */
StateEnum Ready::id() {
  return STATE_READY;
}

UserCommands Ready::userCommands(ExecutionContext *context) {
  return AbstractState::userCommands(context) | CMD_GET_CONFIG |  CMD_GET_LOG | CMD_GET_STAT;
}

EventCandidates Ready::eval(ExecutionContext *context) {
  EventCandidates result = AbstractState::eval(context);
  if (context->op->water.sensorStatus == SENSOR_NOK) {
    result |= EVENT_SENSORS_NOK;
  }
  if (context->op->userCommands & CMD_HELP) {
    result |= EVENT_HELP;
  }
  if (context->op->userCommands & CMD_GET_CONFIG) {
    result |= EVENT_GET_CONFIG;
  }
  if (context->op->userCommands & CMD_GET_LOG) {
    result |= EVENT_GET_LOG;
  }
  if (context->op->userCommands & CMD_GET_STAT) {
    result |= EVENT_GET_STAT;
  }
  return result;
}

StateEnum Ready::transAction(EventEnum event, ExecutionContext *context) {
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
  return AbstractState::transAction(event, context);
}

/*
 * IDLE
 */
StateEnum Idle::id() {
  return STATE_IDLE;
}

UserCommands Idle::userCommands(ExecutionContext *context) {
  return AbstractState::userCommands(context) | CMD_SET_CONFIG | CMD_REC_ON;
}

EventCandidates Idle::eval(ExecutionContext *context) {
  EventCandidates result = AbstractState::eval(context);
  if (context->op->userCommands & CMD_SET_CONFIG) {
    result |= EVENT_SET_CONFIG;
  }
  if (context->op->userCommands & CMD_REC_ON) {
    result |= EVENT_REC_ON;
  }
  return result;
}

StateEnum Idle::transAction(EventEnum event, ExecutionContext *context) {
  if (event == EVENT_SET_CONFIG) {
    context->control->setConfigParam();
    return STATE_SAME;
    
  } else if (event == EVENT_REC_ON) {
    return STATE_RECORDING;
  }
  return AbstractState::transAction(event, context);
}

/*
 * RECORDING
 */
StateEnum Recording::id() {
  return STATE_RECORDING;
}

UserCommands Recording::userCommands(ExecutionContext *context) {
  return AbstractState::userCommands(context) | CMD_REC_OFF;
}


EventCandidates Recording::eval(ExecutionContext *context) {
  EventCandidates result = AbstractState::eval(context);
  if (context->op->userCommands & CMD_REC_OFF) {
    result |= EVENT_REC_OFF;
  }
  return result;
}

StateEnum Recording::transAction(EventEnum event, ExecutionContext *context) {
  if (event == EVENT_REC_OFF) {
    return STATE_IDLE;
  }
  return AbstractState::transAction(event, context);
}

void Recording::entryAction(ExecutionContext *context) {
  context->op->loggingValues = true;
}

void Recording::exitAction(ExecutionContext *context){
  context->op->loggingValues = false;
}

/*
 * STANDBY
 */
StateEnum Standby::id() {
  return STATE_STANDBY;
}

UserCommands Standby::userCommands(ExecutionContext *context) {
  return AbstractState::userCommands(context) | CMD_HEAT_ON;
}

EventCandidates Standby::eval(ExecutionContext *context) {
  EventCandidates result = AbstractState::eval(context);
  if (context->op->userCommands & CMD_HEAT_ON) {
    result |= EVENT_HEAT_ON;
  }
  return result;
}

StateEnum Standby::transAction(EventEnum event, ExecutionContext *context) {
  if (event == EVENT_HEAT_ON) {
    return STATE_HEATING;
  }
  return AbstractState::transAction(event, context);
}

/*
 * HEATING
 */
StateEnum Heating::id() {
  return STATE_HEATING;
}

UserCommands Heating::userCommands(ExecutionContext *context) {
  return AbstractState::userCommands(context) | CMD_HEAT_OFF;
}

EventCandidates Heating::eval(ExecutionContext *context) {
  EventCandidates result = AbstractState::eval(context);
  if (context->op->userCommands & CMD_HEAT_OFF) {
    result |= EVENT_HEAT_OFF;
  }
  if (context->op->water.currentTemp >= context->config->heaterCutOutWaterTemp) {
    result |= EVENT_TEMP_OVER;
  }
  return result;
}

StateEnum Heating::transAction(EventEnum event, ExecutionContext *context) {
  if (event == EVENT_HEAT_OFF) {
    return STATE_STANDBY;
  } else if (event == EVENT_TEMP_OVER) {
    return STATE_OVERHEATED;
  }
  return AbstractState::transAction(event, context);
}

void Heating::entryAction(ExecutionContext *context) {
  context->control->heat(true, context);
}

void Heating::exitAction(ExecutionContext *context){
  context->control->heat(false, context);
}

/*
 * OVERHEATED
 */
StateEnum Overheated::id() {
  return STATE_OVERHEATED;
}

UserCommands Overheated::userCommands(ExecutionContext *context) {
  return AbstractState::userCommands(context) | CMD_RESET;
}

EventCandidates Overheated::eval(ExecutionContext *context) {
  EventCandidates result = AbstractState::eval(context);
  if (context->op->userCommands & CMD_RESET) {
    result |= EVENT_RESET;
  }
  if (context->op->water.currentTemp <= context->config->heaterBackOkWaterTemp) {
    result |= EVENT_TEMP_OK;
  }
  return result;
}

StateEnum Overheated::transAction(EventEnum event, ExecutionContext *context) {
  if (event == EVENT_RESET) {
    return STATE_STANDBY;
    
  } else if (event == EVENT_TEMP_OK) {
    return STATE_HEATING;
  }
  return AbstractState::transAction(event, context);
}

/*
 * STATE AUTOMATON
 */
BoilerStateAutomaton::BoilerStateAutomaton(ExecutionContext *context) {
  this->context = context;
  
  static Init INIT;
  static Idle IDLE;
  static SensorsNOK SENSORS_NOK;
  static Standby STANDBY;
  static Heating HEATING;
  static Overheated OVERHEATED;

  static AbstractState *RECORDING_SUBSTATES[] = {&STANDBY, &HEATING, &OVERHEATED};
  static Recording RECORDING(RECORDING_SUBSTATES, 3);

  static AbstractState *READY_SUBSTATES[] = {&IDLE, &RECORDING};
  static Ready READY(READY_SUBSTATES, 2);

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
  return currentState->userCommands(context);
}
  
EventCandidates BoilerStateAutomaton::evaluate() {
  return currentState->eval(context);
}

void BoilerStateAutomaton::transition(EventEnum event) {
  StateEnum oldState = currentState->id();
  StateEnum newState = currentState->trans(event, context);
  if (newState == STATE_UNDEFINED) {
    if ((event & currentState->illegalTransitionLogged) == 0) { // this illegal event has not been logged at this state
      #ifdef DEBUG_STATE
        Serial.print("DEBUG_STATE: State ");
        Serial.print(currentState->id());
        Serial.print(": log invalid event ");
        Serial.println(event);
      #endif

      context->storage->logMessage(MSG_ILLEGAL_TRANS, currentState->id(), event);
      currentState->illegalTransitionLogged |= event;
    }
    
  } else if (newState != STATE_SAME && oldState != newState) { // we are in a different new state!
    currentState = getState(newState);
    
    // Enter the new state (which can be a composite state but will always end up in a simple state):
    newState = currentState->enter(context);
    currentState = getState(newState);
    
    context->storage->logState(oldState, newState, event);
  }
}

AbstractState *BoilerStateAutomaton::getState(StateEnum id) {
  for(unsigned short i=0; i<NUM_STATES; i++) {
    if (ALL_STATES[i]->id() == id) {
      return ALL_STATES[i];
    }
  }
  assert(false);
}

