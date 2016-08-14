#ifndef BC_SETUP_H_INCLUDED
  #define BC_SETUP_H_INCLUDED
  
  // Uncomment AT MOST ONE (!) of the following lines:
    // #define UNIT_TEST
    // #define BLE_UI
     #define SERIAL_UI
  
  #ifdef UNIT_TEST
    // Comment the following line to prevent execution of STATE tests:
    #define UT_STATE
  #endif
  
#endif
