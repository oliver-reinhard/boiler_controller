#include "unit_test.h"
#ifdef UNIT_TEST
  #include <ArduinoUnit.h>
#endif
#include "storage.h"
#include "control.h"
#include "state.h"
#include "test_state.h"

/*
 * GLOBALS
 */
ConfigParams configParams;
OperationalParams opParams;
ControlActions controlActions;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  #ifndef UNIT_TEST
    Serial.println("Boiler setup MAIN");
    /*
    getConfigParams(&configParams);
    initLog();
    logMessage(MSG_SYSTEM_INIT, 0, 0);
    */
    ExecutionContext ec;
    ec.config = &configParams;
    ec.op = &opParams;
    ec.control = &controlActions;
    
  #endif
 
}

void loop() {
  #ifdef UNIT_TEST
    Test::run();
  #endif
}

