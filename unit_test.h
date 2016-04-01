#ifndef BOILER_UNIT_TEST_H_INCLUDED
  #define BOILER_UNIT_TEST_H_INCLUDED
  
  // Comment the following line to prevent ALL unit tests:
  #define UNIT_TEST
  
  #ifdef UNIT_TEST
  
    // Comment the following line to prevent execution of STORAGE tests:
    //#define TEST_STORAGE
      #define UNIT_TEST_LOG_ENTRIES 5

    // Comment the following line to prevent execution of STORAGE tests:
    #define TEST_STATE
  #endif
#endif
