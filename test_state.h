#include "unit_test.h"
#ifdef TEST_STATE

  #ifndef TEST_STATE_H_INCLUDED
    #define TEST_STATE_H_INCLUDED
    
    #include "storage.h"
    #include "control.h"
  
    class MockControlActions : public ControlActions {
      public:
        // Mocked methods:
        void readSensors(OperationalParams *op);
        void readUserCommands(OperationalParams *op);
    
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

    
    class MockStorage : public Storage {
      public:
        Version version();
        
        void clearConfigParams();
        void getConfigParams(ConfigParams *configParams);
        void updateConfigParams(const ConfigParams *configParams);
        void readConfigParams(ConfigParams *configParams);
        
        void resetLog();
        void initLog();
        Timestamp logValues(Temperature water, Temperature ambient, Flags flags);
        Timestamp logState(StateID previous, StateID current, EventID event);
        Timestamp logMessage(MessageID id, short param1, short param2);
      
        // Mock counters:
        unsigned short logValuesCount = 0;
        unsigned short logStateCount = 0;
        unsigned short logMessageCount = 0;
        
        unsigned short totalInvocations();
        void resetCounters();
    };

  #endif

#endif