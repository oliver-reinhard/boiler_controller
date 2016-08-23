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
    EVENT_NONE          = 0,      // 1
    EVENT_READY         = 0x1,    // 2
    EVENT_SENSORS_NOK   = 0x2,    // 3
    EVENT_INFO          = 0x4,    // 4
    EVENT_CONFIG_MODIFY = 0x8,    // 5     (8)
    EVENT_CONFIG_RESET  = 0x10,   // 6    (16)
    EVENT_REC_ON        = 0x20,   // 7    (32)
    EVENT_REC_OFF       = 0x40,   // 8    (64)
    EVENT_HEAT_ON       = 0x80,   // 9   (128)
    EVENT_HEAT_OFF      = 0x100,  // 10  (256)
    EVENT_HEAT_RESET    = 0x200,  // 11  (512)
    EVENT_TEMP_OVER     = 0x400,  // 12 (1024)
    EVENT_TEMP_OK       = 0x800   // 13 (2048)
  } EventEnum;

  const uint16_t NUM_EVENTS = 13;
  
  /*
   * Maps state-automaton events to user commands.
   * Note: not all events have corresponding user commands (=> CMD_NONE), but all user commands map to an event
   */
  const UserCommands EVENT_CMD_MAP [NUM_EVENTS] = {
    CMD_NONE,                                                                               // 1 - EVENT_NONE
    CMD_NONE,                                                                               // 2 - EVENT_READY
    CMD_NONE,                                                                               // 3 - EVENT_SENSORS_NOK
    CMD_INFO_HELP | CMD_INFO_STAT | CMD_INFO_CONFIG | CMD_INFO_LOG,                         // 4 - EVENT_INFO
    CMD_CONFIG_SET_VALUE | CMD_CONFIG_SWAP_IDS | CMD_CONFIG_CLEAR_IDS | CMD_CONFIG_ACK_IDS, // 5 - EVENT_CONFIG_MODIFY
    CMD_CONFIG_RESET_ALL,                                                                   // 6 - EVENT_CONFIG_RESET
    CMD_REC_ON,                                                                             // 7 - EVENT_REC_ON
    CMD_REC_OFF,                                                                            // 8 - EVENT_REC_OFF
    CMD_HEAT_ON,                                                                            // 9 - EVENT_HEAT_ON
    CMD_HEAT_OFF,                                                                           // 10 - EVENT_HEAT_OFF
    CMD_HEAT_RESET,                                                                         // 11 - EVENT_HEAT_RESET
    CMD_NONE,                                                                               // 12 - EVENT_TEMP_OVER
    CMD_NONE                                                                                // 13 - EVENT_TEMP_OK
  };
  
  /*
   * All events (except EVENT_NONE) ordered by descending priority, i.e. most urgent first.
   */
  const EventEnum EVENT_PRIORITIES [NUM_EVENTS] = {
    EVENT_TEMP_OVER,     // 1
    EVENT_TEMP_OK,       // 2
    EVENT_READY,         // 3
    EVENT_SENSORS_NOK,   // 4
    EVENT_HEAT_OFF,      // 5
    EVENT_HEAT_ON,       // 6
    EVENT_REC_OFF,       // 7
    EVENT_REC_ON,        // 8
    EVENT_HEAT_RESET,    // 9
    EVENT_CONFIG_RESET,  // 10
    EVENT_CONFIG_MODIFY, // 11
    EVENT_INFO,          // 12
    EVENT_NONE           // 13
  };
  
  /*
   * The set of events accepted in a given stated as or'ed ("|") together.
   */
  typedef uint16_t AcceptedEvents;
  
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
      AbstractState() { }

      /*
       * Mandatory initialisation. This method has to be invoked before any other methods on a state are invoked.
       */
      void init(ExecutionContext *context);

      /*
       * The non-object ID of the state.
       */
      virtual StateEnum id() = 0;

      /*
       * Calculates and returns the set of user commands supported by the current state at the time of invocation of this method.
       * 
       * The default implementation invokes the containing state's acceptedUserEvents() method first, then adds its own commands to the result.
       * 
       * Returns CMD_NONE if there should be no user commands available at the time.
       */
      virtual AcceptedEvents acceptedUserEvents();

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
      AbstractSimpleState() : AbstractState() { };
      
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
      AbstractCompositeState()  : AbstractState() { }
      
      void init(ExecutionContext *context, AbstractState **substates, uint16_t numSubstates);
      
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
      StateEnum id() { return STATE_INIT; }
      // No user commands for this state.
      EventCandidates eval();
      
    protected:
      StateEnum transAction(EventEnum event);
  };
  
  
  class SensorsNOK : public AbstractSimpleState {
    public:
      StateEnum id() { return STATE_SENSORS_NOK; }
      AcceptedEvents acceptedUserEvents();
      
    protected:
      StateEnum transAction(EventEnum event);
  };

  
  class Ready : public AbstractCompositeState {
    public:      
      StateEnum id() { return STATE_READY; }
      AcceptedEvents acceptedUserEvents();
      
    protected:
      StateEnum transAction(EventEnum event);
  };

  
  class Idle : public AbstractSimpleState {
    public:
      StateEnum id() { return STATE_IDLE; }
      AcceptedEvents acceptedUserEvents();
      
    protected:
      StateEnum transAction(EventEnum event);
  };

  
  class Recording : public AbstractCompositeState {
    public:
      StateEnum id() { return STATE_RECORDING; }
      AcceptedEvents acceptedUserEvents();
      
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
      StateEnum id() { return STATE_STANDBY; }
      AcceptedEvents acceptedUserEvents();
      
    protected:
      StateEnum transAction(EventEnum event);
  };

  
  class Heating : public AbstractSimpleState {
    public:
      StateEnum id() { return STATE_HEATING; }
      AcceptedEvents acceptedUserEvents();
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
      StateEnum id() { return STATE_OVERHEATED; }
      AcceptedEvents acceptedUserEvents();
      EventCandidates eval();
      
    protected:
      StateEnum transAction(EventEnum event);
  };

  
  /*
   * STATE AUTOMATON
   */
  class BoilerStateAutomaton {
    public:
      void init(ExecutionContext *context);
      
      /*
       * Returns the current state.
       */
      AbstractState *state();

      /*
       * Returns the set of user commands supported by the current state of the automaton.
       */
      UserCommands acceptedUserCommands();
    
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

      /*
       * Map a user command to its corresponding event using EVENT_CMD_MAP.
       * @return EVENT_NONE if no mapping is found
       */
      EventEnum commandToEvent(UserCommandEnum command);
    
    private:
      ExecutionContext *context;
      
      Init INIT;
      Idle IDLE;
      SensorsNOK SENSORS_NOK;
      Standby STANDBY;
      Heating HEATING;
      Overheated OVERHEATED;
      Recording RECORDING;
      AbstractState *RECORDING_SUBSTATES[3] = {&STANDBY, &HEATING, &OVERHEATED};
      Ready READY;
      AbstractState *READY_SUBSTATES[2] = {&IDLE, &RECORDING};

      AbstractState *ALL_STATES[STATE_OVERHEATED + 1];
      AbstractState *currentState;
      
      AbstractState *getState(StateEnum id);

  #ifdef UNIT_TEST
    public:
  #endif
      
      /*
       * Map a set of events ot their corresponding user commands using EVENT_CMD_MAP.
       * @return CMD_NONE if no mapping is found
       */
      UserCommands eventsToCommands(AcceptedEvents events);
  };
  
#endif
