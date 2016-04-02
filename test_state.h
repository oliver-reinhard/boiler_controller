#include "unit_test.h"
#ifdef TEST_STATE

  #ifndef TEST_STATE_H_INCLUDED
    #define TEST_STATE_H_INCLUDED
    
    #include "control.h"
  
    class MockControlActions : public ControlActions {
      public:
        // Mocked methods:
        void readSensors(OperationalParams *op);
    
        void heat(boolean on, OperationalParams *op);
    
        void logValues(boolean on, OperationalParams *op);
    
        void setConfigParam();
        void getLog();
        void getConfig();
        void getStat();
        
        // Mock counters:
        unsigned short heatTrueCount = 0;
        unsigned short heatFalseCount = 0;
        unsigned short logValuesTrueCount = 0;
        unsigned short logValuesFalseCount = 0;
        unsigned short setConfigParamCount = 0;
        unsigned short getLogCount = 0;
        unsigned short getConfigCount = 0;
        unsigned short getStatCount = 0;
        
        unsigned short totalInvocations();

        void resetCounters();
    };

  #endif

#endif
