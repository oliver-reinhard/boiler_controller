#ifdef TEST_STATE

  #ifndef TEST_STATE_H_INCLUDED
    #define TEST_STATE_H_INCLUDED
    
    #include "storage.h"
    #include "control.h"
  
    class MockControlActions : public ControlActions {
      public:
        // Mocked methods:
        void setupSensors(ControlContext *context);
        void initSensorReadout(ControlContext *context);
        void completeSensorReadout(ControlContext *context);
    
        void heat(boolean on, ControlContext *context);
    
        void setConfigParam();
        void requestHelp();
        void requestLog();
        void requestConfig();
        void requestStat();
        
        // Mock counters:
        unsigned short heatTrueCount = 0;
        unsigned short heatFalseCount = 0;
        unsigned short setConfigParamCount = 0;
        unsigned short requestHelpCount = 0;
        unsigned short requestLogCount = 0;
        unsigned short requestConfigCount = 0;
        unsigned short requestStatCount = 0;
        
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
