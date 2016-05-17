#ifndef BOILER_BC_SETUP_H_INCLUDED
  #define BOILER_BC_SETUP_H_INCLUDED
  
  // Comment the following line to prevent ALL unit tests:
  //#define UNIT_TEST
  
  #ifndef UNIT_TEST
    // Comment the following line to get the SERIAL UI:
    //#define BLE_UI
  #endif
  
  #ifdef UNIT_TEST
    
    // Comment the following line to prevent execution of STORAGE tests:
    #define UT_STORAGE
    #define UNIT_TEST_LOG_ENTRIES 5

    // Comment the following line to prevent execution of STATE tests:
    //#define UT_STATE
    
    // Do NOT COMMENT the following line:
    #define BLE_UI
    
  #endif
  
#endif
