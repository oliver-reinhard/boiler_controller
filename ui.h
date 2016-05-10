#ifndef BOILER_UI_H_INCLUDED
  #define BOILER_UI_H_INCLUDED

  #include "control.h"
  #include "state.h"

  /*
   * Reads one user command and stores them in context->op.userCommand.
   */
  void readUserCommand(ControlContext *context);
  void processReadWriteRequests(ReadWriteRequests requests, ControlContext *context, BoilerStateAutomaton *automaton);

#endif
