#ifndef BOILER_UI_H_INCLUDED
  #define BOILER_UI_H_INCLUDED

  #include "control.h"
  #include "state.h"

  /*
   * Reads user commands and stores them in context->op.userCommands.
   */
  void readUserCommands(ControlContext *context);
  void processInfoRequests(InfoRequests requests, ControlContext *context, BoilerStateAutomaton *automaton);

#endif
