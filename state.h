#ifndef BC_STATE_H_INCLUDED
  #define BC_STATE_H_INCLUDED
  
  #include "control.h"
  
  typedef enum {
    MSG_ILLEGAL_TRANS = 30,   // State [state]: illegal transition attemt (event [event])
    MSG_UNKNOWN_STATE = 31    // State [state] has not been defined
  } StateMessageEnum;
  
  typedef enum  {
    STATE_UNDEFINED = -2,
    STATE_SAME = -1,
    //
    STATE_INIT = 0,
    STATE_SENSORS_NOK = 1,
    STATE_READY = 2, 
    STATE_IDLE = 3, 
    STATE_RECORDING = 4,
    STATE_STANDBY = 5, 
    STATE_HEATING = 6, 
    STATE_OVERHEATED = 7
  } StateEnum;

  const uint16_t NUM_STATES = STATE_OVERHEATED + 1;
  
  typedef enum { 
    EVENT_NONE = 0,
    EVENT_READY = 0x1,        // 1
    EVENT_SENSORS_NOK = 0x2,  // 2
    EVENT_SET_CONFIG = 0x4,   // 3     (4) user command
    EVENT_RESET_CONFIG = 0x8, // 4     (8) user command
    EVENT_REC_ON = 0x10,      // 5    (16) user command
    EVENT_REC_OFF = 0x20,     // 6    (32) user command
    EVENT_HELP = 0x40,        // 7    (64) user command
    EVENT_GET_CONFIG = 0x80,  // 8   (128) user command
    EVENT_GET_LOG = 0x100,    // 9   (256) user command
    EVENT_GET_STAT = 0x200,   // 10  (512) user command
    EVENT_HEAT_ON = 0x400,    // 11 (1024) user command
    EVENT_HEAT_OFF = 0x800,   // 12 (2048) user command
    EVENT_TEMP_OVER = 0x1000, // 13 (4096)
    EVENT_TEMP_OK = 0x2000,   // 14 (8192)
    EVENT_HEAT_RESET = 0x4000 // 15 (16384) user command
  } EventEnum;

  const uint16_t NUM_EVENTS = 15;
  
  /*
   * All events (except EVENT_NONE) ordered by descending priority, i.e. most urgent first.
   */
  const EventEnum EVENT_PRIORITIES[NUM_EVENTS] = {
    EVENT_TEMP_OVER,    // 1
    EVENT_TEMP_OK,      // 2
    EVENT_READY,        // 3
    EVENT_SENSORS_NOK,  // 4
    EVENT_HEAT_OFF,     // 5
    EVENT_HEAT_ON,      // 6
    EVENT_REC_OFF,      // 7
    EVENT_REC_ON,       // 8
    EVENT_HEAT_RESET,   // 9
    EVENT_GET_STAT,     // 10
    EVENT_HELP,         // 11
    EVENT_SET_CONFIG,   // 12
    EVENT_RESET_CONFIG, // 13
    EVENT_GET_CONFIG,   // 14
    EVENT_GET_LOG       // 15
  };
    
  /*
   * The result of the evaluation of the operational parameters by a state.
   * Denotes all possible events that result from the evaluation by a state as or'ed ("|") together.
   */
  typedef uint16_t EventCandidates;
  

  class ExecutionContext : public ControlContext {
    public:
      ControlActions *control;
  };


  /*
   * The mother of all states. 
   * Note: All state classes are stateless in that they do not store any values during or after state changes or as a result of executing actions.
   */
  class AbstractState {
    public:
      /*
       * The containing state, or NULL if none.
       */
      AbstractState *containingState = NULL;
      
      /*
       * Records whether an illegal transition has been logged at this state in order to avoid excessive logging.
       * Note: the presence of an event bit in the set EventCandidates means the attempt has been logged and will not be logged again
       * at this state for the lifetime of the state object.
       */
      EventCandidates illegalTransitionLogged = EVENT_NONE;

      /*
       * Constructor.
       */
      AbstractState(ExecutionContext *context) {
         this->context = context;
  Serial.print("AbstractState=");
  Serial.println(this->context->op->water.sensorStatus, HEX);
      }

      /*
       * The non-object ID of the state.
       */
      virtual StateEnum id() = 0;

      /*
       * Calculates and returns the set of user commands supported by the current state at the time of invocation of this method.
       * 
       * The default implementation invokes the containing state's userCommands() method first, then adds its own commands to the result.
       * 
       * Returns CMD_NONE if there should be no user commands available at the time.
       */
      virtual UserCommands userCommands();

      /*
       * Enters a state by invoking its entryAction() method, then triggers the enter() method of its initial substate (if any).
       * Note: enter methods are always invoked *down* the containment chain, i.e. containing state first, then its substates until a simple state is reached.
       * 
       * Returns the actual new state, which is always a simple state; if "this" is a composite state then the new state is its initial substate and so on.
       */
      virtual StateEnum enter() = 0;
      
      /*
       * Checks every event condition and returns those events ready for transitioning at the time of invocation of this method. 
       * Note: this method does not actually perform a transition, it merely calculates the options for transitioning. It is  left to the caller
       * to pick the event with the highest priority by his own definition and trigger the transition.
       * 
       * The default implementation invokes the containing state's eval() method first, then adds its own evaluation to the result.
       * 
       * Returns TRANS_NONE if there should not be a state change.
       */
      virtual EventCandidates eval();
      
      /*
       * Handels the event and executes the transistion. If this state can't handle the event it delegates to the containing state's trans() method.
       * Handling the event consists of invoking its transAction() method (which may execute commands and computes the next state).
       * Note: this method stays at the same state or transitions *out* of it but does *not yet enter* the next state.
       * 
       * Returns STATE_UNDEFINED if event wasn't handled.
       */
      virtual StateEnum trans(EventEnum event) = 0;
     
      /*
       * Performes the exit tasks of "this", then invokes the containing state's exit method.
       * Note: exit methods are always invoked *up* the containment chain, i.e. simple state first, then its containing state(s).
       */
      virtual void exit(EventEnum event, StateEnum next) = 0;
  
    protected:
      /*
       * "Connection" to the world outside the state.
       */
      ExecutionContext *context;
          
      /*
       * Executes the transition actions for this event (if any), calculates and returns the next state.
       * 
       * Returns STATE_UNDEFINED if event couldn't be handled at this level.
       */
      virtual StateEnum transAction(EventEnum event);

      /*
       * Executes the action(s) that are always performed when this state is entered.
       */
      virtual void entryAction();
      
      /*
       * Executes the action(s) that are always performed when this state is truly exited.
       */
      virtual void exitAction();
  };


  class AbstractSimpleState : public AbstractState {
    public:
      AbstractSimpleState(ExecutionContext *context) : AbstractState(context) { };
      
      virtual StateEnum enter();
      
      /*
       * If the next state is different from "this" then the exit() method is invoked, which may directly trigger exit() at containing states.
       */
      virtual StateEnum trans(EventEnum event);

    protected:
      virtual void exit(EventEnum event, StateEnum next);
  };

  
  class AbstractCompositeState : public AbstractState {
    public:
      AbstractCompositeState(ExecutionContext *context, AbstractState **substates, uint16_t numSubstates);
      
      AbstractState *initialSubstate();
      
      virtual StateEnum enter();
      
      /*
       * Does not invoke the exit action (exits are triggered by simple states and are then performed "up" the containment hiearchy).
       */
      virtual StateEnum trans(EventEnum event);
      
    protected:
      /*
       * If the next state is a simple state not contained by "this" then the exitAction() method is invoked (else we won't leave "this").
       */      
      virtual void exit(EventEnum event, StateEnum next);
      
    protected:   
      AbstractState **substates;
      uint16_t numSubstates;
  };
  
  
  class Init : public AbstractSimpleState {
    public:
      Init(ExecutionContext *context) : AbstractSimpleState(context) { };
      StateEnum id() { return STATE_INIT; }
      // No user commands for this state.
      EventCandidates eval();
      
    protected:
      StateEnum transAction(EventEnum event);
  };
  
  
  class SensorsNOK : public AbstractSimpleState {
    public:
      SensorsNOK(ExecutionContext *context) : AbstractSimpleState(context) { };
      StateEnum id() { return STATE_SENSORS_NOK; }
      UserCommands userCommands();
      EventCandidates eval();
      
    protected:
      StateEnum transAction(EventEnum event);
  };

  
  class Ready : public AbstractCompositeState {
    public:
      Ready(ExecutionContext *context, AbstractState **substates, uint16_t numSubstates) : AbstractCompositeState(context, substates, numSubstates) { };
      
      StateEnum id() { return STATE_READY; }
      UserCommands userCommands();
      EventCandidates eval();
      
    protected:
      StateEnum transAction(EventEnum event);
  };

  
  class Idle : public AbstractSimpleState {
    public:
      Idle(ExecutionContext *context) : AbstractSimpleState(context) { };
      StateEnum id() { return STATE_IDLE; }
      UserCommands userCommands();
      EventCandidates eval();
      
    protected:
      StateEnum transAction(EventEnum event);
  };

  
  class Recording : public AbstractCompositeState {
    public:
      Recording(ExecutionContext *context, AbstractState **substates, uint16_t numSubstates) : AbstractCompositeState(context, substates, numSubstates) { };
      
      StateEnum id() { return STATE_RECORDING; }
      UserCommands userCommands();
      EventCandidates eval();
      
    protected:
      StateEnum transAction(EventEnum event);
      
      /*
       * Turn value logging ON (entry) / OFF (exit)
       */
      void entryAction();
      void exitAction();
  };

  
  class Standby : public AbstractSimpleState {
    public:
      Standby(ExecutionContext *context) : AbstractSimpleState(context) { };
      StateEnum id() { return STATE_STANDBY; }
      UserCommands userCommands();
      EventCandidates eval();
      
    protected:
      StateEnum transAction(EventEnum event);
  };

  
  class Heating : public AbstractSimpleState {
    public:
      Heating(ExecutionContext *context) : AbstractSimpleState(context) { };
      StateEnum id() { return STATE_HEATING; }
      UserCommands userCommands();
      EventCandidates eval();
      
    protected:
      StateEnum transAction(EventEnum event);
      
      /*
       * Turn heater ON (entry) / OFF (exit)
       */
      void entryAction();
      void exitAction();
  };

  
  class Overheated : public AbstractSimpleState {
    public:
      Overheated(ExecutionContext *context) : AbstractSimpleState(context) { };
      StateEnum id() { return STATE_OVERHEATED; }
      UserCommands userCommands();
      EventCandidates eval();
      
    protected:
      StateEnum transAction(EventEnum event);
  };

  
  /*
   * STATE AUTOMATON
   */
  class BoilerStateAutomaton {
    public:
      BoilerStateAutomaton(ExecutionContext *context);
      
      /*
       * Returns the current state.
       */
      AbstractState *state();

      /*
       * Returns the set of user commands supported by the current state of the automaton.
       */
      UserCommands userCommands();
    
      /*
       * Returns all possible transitions from the current state, or EVENT_NONE if there is none. 
       * 
       * PREREQUISITE: the ExecutionContext has been updated very recently, including sensor readout
       * 
       * Note: The result EVENT_NONE may imply that a user command is not supported at the given state.
       */
      EventCandidates evaluate();
    
      /*
       * Execute the transition for the given event from the current state. This includes entering the new state, if there is a transition to a new state at all.
       */
      void transition(EventEnum event);
    
    private:
      ExecutionContext *context;
      
      AbstractState *ALL_STATES[STATE_OVERHEATED + 1];
      AbstractState *currentState;
      
      AbstractState *getState(StateEnum id);
  };
  
#endif
