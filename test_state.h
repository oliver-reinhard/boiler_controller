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
        unsigned short heatTrueCount;
        unsigned short heatFalseCount;
        unsigned short logValuesTrueCount;
        unsigned short logValuesFalseCount;
        unsigned short setConfigParamCount;
        unsigned short getLogCount;
        unsigned short getConfigCount;
        unsigned short getStatCount;
        
        unsigned short totalInvocations();

        void resetCounters();
    };

  #endif

#endif
