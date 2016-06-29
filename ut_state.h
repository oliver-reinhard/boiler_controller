#include "bc_setup.h"
#ifdef UT_STATE

  #ifndef UT_STATE_H_INCLUDED
    #define UT_STATE_H_INCLUDED
    
    #include "log.h"
    #include "control.h"
  
    class MockControlActions : public ControlActions {
      public:
        MockControlActions(ControlContext *context) : ControlActions(context) { }
        
        // Mocked methods:
        void setupSensors() { }
        void initSensorReadout() { }
        void completeSensorReadout() { }
    
        void heat(boolean on);
    
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

    
    class MockConfig : public ConfigParams {
      public:
        LayoutVersion version()  { return EEPROM_LAYOUT_VERSION; }
        void clear() { }
        void load()  { }
        void save()  { }
        void initParams(boolean &updated) {
          ConfigParams::initParams(updated);
        }
    };
    
    class MockLog : public Log {
      public:
        MockLog() : Log(0L) {}
        void clear() { }
        void init()  { }
        Timestamp logMessage(MessageID msg, int16_t param1, int16_t param2);
        Timestamp logValues(Temperature water, Temperature ambient, Flags flags);
        Timestamp logState(StateID previous, StateID current, EventID event);
      
        // Mock counters:
        uint16_t logValuesCount = 0;
        uint16_t logStateCount = 0;
        uint16_t logMessageCount = 0;
        
        uint16_t totalInvocations();
        void resetCounters();
    };

  #endif
#endif
