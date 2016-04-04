#include "control.h"

 void ControlActions::readSensors(OperationalParams *op) {
    if (op == NULL) { } // prevent warning "unused parameter ..."
  }

 void ControlActions::readUserCommands(OperationalParams *op) {
    if (op == NULL) { } // prevent warning "unused parameter ..."
 }

  void ControlActions::heat(boolean on, OperationalParams *op) {
    op->heating = on;
    if (on) {
      // turn heater on
    } else {
      // turn heater off
    }
  }

  void ControlActions::logValues(boolean on, OperationalParams *op) {
    op->loggingValues = on;
    if (on) {
      // turn logging on
    } else {
      // turn logging off
    }
  }

  void ControlActions::setConfigParam() {
    // TODO
  }
  
  void ControlActions::getLog() {
    // TODO
  }
  
  void ControlActions::getConfig() {
    // TODO
  }
  
  void ControlActions::getStat() {
    // TODO
  }
