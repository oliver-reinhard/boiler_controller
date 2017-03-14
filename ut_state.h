#include "bc_setup.h"
#ifdef UT_STATE

  #ifndef UT_STATE_H_INCLUDED
    #define UT_STATE_H_INCLUDED
    
    #include "log.h"
    #include "control.h"
  
    class MockControlActions : public ControlActions {
      public:
        MockControlActions(ControlContext *context, UserFeedback *feedback) : ControlActions(context, feedback) { }
         
        // Mocked methods:
        void modifyConfig();
        
        uint8_t setupSensors() { return 0; }
        void initSensorReadout() { }
        void completeSensorReadout() { }
    
        void heat(boolean on);
        
        // Mock counters:
        uint16_t heatTrueCount = 0;
        uint16_t heatFalseCount = 0;
        uint16_t modifyConfigCount = 0;
        
        uint16_t totalInvocations();
        void resetCounters();
    };

    
    class MockConfig : public ConfigParams {
      public:
        MockConfig() : ConfigParams(NULL) { }
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
