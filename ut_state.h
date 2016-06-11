#include "bc_setup.h"
#ifdef UT_STATE

  #ifndef UT_STATE_H_INCLUDED
    #define UT_STATE_H_INCLUDED
    
    #include "store.h"
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
        uint16_t heatTrueCount = 0;
        uint16_t heatFalseCount = 0;
        uint16_t setConfigParamCount = 0;
        uint16_t requestHelpCount = 0;
        uint16_t requestLogCount = 0;
        uint16_t requestConfigCount = 0;
        uint16_t requestStatCount = 0;
        
        uint16_t totalInvocations();
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
        Timestamp logMessage(MessageEnum msg, int16_t param1, int16_t param2);
      
        // Mock counters:
        uint16_t logValuesCount = 0;
        uint16_t logStateCount = 0;
        uint16_t logMessageCount = 0;
        
        uint16_t totalInvocations();
        void resetCounters();
    };

  #endif

#endif
