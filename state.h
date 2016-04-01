#ifndef BOILER_STATE_H_INCLUDED
  #define BOILER_STATE_H_INCLUDED
  
  #include "log.h"
  #include "control.h"
  
  typedef enum  {
    STATE_UNDEFINED = -2,
    STATE_SAME = -1,
    //
    STATE_INIT = 0,
    STATE_READY = 1, 
    STATE_IDLE = 2, 
    STATE_RECORDING = 3,
    STATE_STANDBY = 4, 
    STATE_HEATING = 5, 
    STATE_OVERHEATED = 6
  } StateEnum;

  const unsigned short NUM_STATES = STATE_OVERHEATED + 1;
  
  typedef enum { 
    EVENT_NONE = 0,
    EVENT_READY = 0x1, 
    EVENT_SET_CONFIG = 0x2, 
    EVENT_REC_ON = 0x4, 
    EVENT_REC_OFF = 0x8,
    EVENT_GET_CONFIG = 0x10,
    EVENT_GET_LOG = 0x20,
    EVENT_GET_STAT = 0x40,
    EVENT_HEAT_ON = 0x80,
    EVENT_HEAT_OFF = 0x100,
    EVENT_TEMP_OVER = 0x200,
    EVENT_TEMP_OK = 0x400,
    EVENT_RESET = 0x800
  } EventEnum;

  /**
   * The result of the evaluation of the operational parameters by a state.
   * Denotes all possible events that result from the evaluation by a state as or'ed ("|") together.
   */
  typedef byte EventCandidates;

  struct ExecutionContext {
    const ConfigParams *config;
    OperationalParams *op;
    ControlActions *control;
  };


  /*
   * The mother of all states. 
   * Note: All state classes are stateless in that they do not store any values during or after state changes or as a result of executing actions.
   */
  class AbstractState {
    public:
      
      /**
       * The containing state, or NULL if none.
       */
      AbstractState *containingState = NULL;

      /*
       * The non-object ID of the state.
       */
      virtual StateEnum id() = 0;

      /**
       * Enters a state by invoking its entryAction() method, then triggers the enter() method of its initial substate (if any).
       * Note: enter methods are always invoked *down* the containment chain, i.e. containing state first, then its substates until a simple state is reached.
       * 
       * Returns the actual new state, which is always a simple state; if "this" is a composite state then the new state is its initial substate and so on.
       */
      virtual StateEnum enter(ExecutionContext *context) = 0;
      
      /*
       * Invokes the containing state's eval() method first, then adds its own evaluation to the result.
       * Note: this method does not actually perform a transition, it merely calculates the options for transitioning.
       * 
       * Returns TRANS_NONE if there should not be a state change.
       */
      virtual EventCandidates eval(ExecutionContext *context);
      
      /*
       * Handels the event and executes the transistion. If this state can't handle the event it delegates to the containing state's trans() method.
       * Handling the event consists of invoking its transAction() method (which may execute commands and computes the next state).
       * Note: this method stays at the same state or transitions *out* of it but does *not yet enter* the next state.
       * 
       * Returns STATE_UNDEFINED if event wasn't handled.
       */
      virtual StateEnum trans(EventEnum event, ExecutionContext *context) = 0;
     
      /*
       * Performes the exit tasks of "this", then invokes the containing state's exit method.
       * Note: exit methods are always invoked *up* the containment chain, i.e. simple state first, then its containing state(s).
       */
      virtual void exit(EventEnum event, StateEnum next, ExecutionContext *context) = 0;
  
    protected:     
      /*
       * Executes the transition actions for this event (if any), calculates and returns the next state.
       * 
       * Returns TRANS_NONE if event couldn't be handled at this level.
       */
      virtual StateEnum transAction(EventEnum event, ExecutionContext *context);

      /*
       * Executes the action(s) that are always performed when this state is entered.
       */
      virtual void entryAction(ExecutionContext *context);
      
      /*
       * Executes the action(s) that are always performed when this state is truly exited.
       */
      virtual void exitAction(ExecutionContext *context);
  };


  class AbstractSimpleState : public AbstractState {
    public:
      virtual StateEnum enter(ExecutionContext *context);
      
      /*
       * If the next state is different from "this" then the exit() method is invoked, which may directly trigger exit() at containing states.
       */
      virtual StateEnum trans(EventEnum event, ExecutionContext *context);

    protected:
      virtual void exit(EventEnum event, StateEnum next, ExecutionContext *context);
  };

  
  class AbstractCompositeState : public AbstractState {
    public:
      AbstractCompositeState(AbstractState **substates, unsigned short numSubstates);
      
      AbstractState *initialSubstate();
      
      virtual StateEnum enter(ExecutionContext *context);
      
      /*
       * Does not invoke the exit action (exits are triggered by simple states and are then performed "up" the containment hiearchy).
       */
      virtual StateEnum trans(EventEnum event, ExecutionContext *context);
      
    protected:
      /*
       * If the next state is a simple state not contained by "this" then the exitAction() method is invoked (else we won't leave "this").
       */      
      virtual void exit(EventEnum event, StateEnum next, ExecutionContext *context);
      
    protected:   
      AbstractState **substates;
      unsigned short numSubstates;
  };
  
  
  class Init : public AbstractSimpleState {
    public:
      StateEnum id();
      EventCandidates eval(ExecutionContext *context);
      
    protected:
      StateEnum transAction(EventEnum event, ExecutionContext *context);
  };

  
  class Ready : public AbstractCompositeState {
    public:
      Ready(AbstractState **substates, unsigned short numSubstates) : AbstractCompositeState(substates, numSubstates) { };
      
      StateEnum id();
      EventCandidates eval(ExecutionContext *context);
      
    protected:
      StateEnum transAction(EventEnum event, ExecutionContext *context);
  };

  
  class Idle : public AbstractSimpleState {
    public:
      StateEnum id();
      EventCandidates eval(ExecutionContext *context);
      
    protected:
      StateEnum transAction(EventEnum event, ExecutionContext *context);
  };

  
  class Recording : public AbstractCompositeState {
    public:
      Recording(AbstractState **substates, unsigned short numSubstates) : AbstractCompositeState(substates, numSubstates) { };
      
      StateEnum id();
      EventCandidates eval(ExecutionContext *context);
      
    protected:
      StateEnum transAction(EventEnum event, ExecutionContext *context);
      
      /*
       * Turn value logging ON (entry) / OFF (exit)
       */
      void entryAction(ExecutionContext *context);
      void exitAction(ExecutionContext *context);
  };

  
  class Standby : public AbstractSimpleState {
    public:
      StateEnum id();
      EventCandidates eval(ExecutionContext *context);
      
    protected:
      StateEnum transAction(EventEnum event, ExecutionContext *context);
  };

  
  class Heating : public AbstractSimpleState {
    public:
      StateEnum id();
      EventCandidates eval(ExecutionContext *context);
      
    protected:
      StateEnum transAction(EventEnum event, ExecutionContext *context);
      
      /*
       * Turn heater ON (entry) / OFF (exit)
       */
      void entryAction(ExecutionContext *context);
      void exitAction(ExecutionContext *context);
  };

  
  class Overheated : public AbstractSimpleState {
    public:
      StateEnum id();
      EventCandidates eval(ExecutionContext *context);
      
    protected:
      StateEnum transAction(EventEnum event, ExecutionContext *context);
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
       * Returns all possible transitions from the current state, or EVENT_NONE if there is none. 
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
