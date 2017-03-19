#include "BC_Controller.h"

// Uncomment AT MOST ONE (!) of the following lines:
   #define BLE_UI
  //#define CONSOLE_UI

// Comment the following line to prevent waiting for the serial connection:
  #define WAIT_FOR_SERIAL

     
#if defined CONSOLE_UI
  #include "BC_UI_Console.h"
#elif defined BLE_UI
  #include "BC_UI_BLE.h"
#endif

BC_Controller controller = BC_Controller();

#if defined BLE_UI
  BLEUI ui = BLEUI();
#elif defined CONSOLE_UI
  ConsoleUI ui = ConsoleUI();
#endif


void setup() {
  #ifdef WAIT_FOR_SERIAL
    Serial.begin(115200);
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
    }
  #endif
  Serial.println(F("Controller Starting up ..."));

  controller.init(&ui);
  
  Serial.println(F("Controller ready."));
}

void loop() {
  controller.loop();
}

