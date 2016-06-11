#ifndef BOILER_BC_SETUP_H_INCLUDED
  #define BOILER_BC_SETUP_H_INCLUDED
  
  // Comment the following line to prevent ALL unit tests:
  //#define UNIT_TEST

  // Comment to disable assert() calls:
  //#define NDEBUG
  
  #ifndef UNIT_TEST
    // Comment ONE (!) of the following lines:
    #define BLE_UI
    //#define SERIAL_UI
  #endif
  
  #ifdef UNIT_TEST
    
    // Comment the following line to prevent execution of STORAGE tests:
    //#define UT_STORE
    #define UNIT_TEST_LOG_ENTRIES 5

    // Comment the following line to prevent execution of STATE tests:
    #define UT_STATE
    
  #endif
  
#endif
