#include <ArduinoUnit.h>
#include "Configuration.h"

//#define DEBUG_UT_CONFIG

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect.
  }
  //Test::min_verbosity = TEST_VERBOSITY_ALL;
}

void loop() {
  Test::run();
}

// ------ Unit Tests --------

#define EEPROM_OFFSET    50L
#define TEST_DATA_VERSION 12
#define TEST_DATA_LENGTH 4
#define BLOB_BASE_VALUE  222
#define BLOB_NEW_VALUE   111

class TestConfigParams : public AbstractConfigParams {
  public:
    TestConfigParams() : AbstractConfigParams(EEPROM_OFFSET, TEST_DATA_VERSION) { };

    // Actual configuration parameters:
    // (is public for test verification purposes)
    byte blob[TEST_DATA_LENGTH];

    uint16_t memSize() { return sizeof(*this); };
    
    void initParams(boolean &updated) {
      updated = false;
      for(uint16_t i = 0; i < TEST_DATA_LENGTH; i++) {
        if (blob[i] == 0) {
          blob[i] = BLOB_BASE_VALUE + i;
          updated = true;
        }
      }
    }
};


test(configuration) {
  TestConfigParams config = TestConfigParams();
  assertEqual(config.memSize(), sizeof(TestConfigParams));
  assertEqual(config.eepromSize(), 2*sizeof(uint8_t) + TEST_DATA_LENGTH);
  
  config.clear();
  assertEqual(config.version(), 0);
 
  config.load();
  #ifdef DEBUG_UT_CONFIG
    config.print();
  #endif
  assertEqual(config.version(), TEST_DATA_VERSION);

  assertEqual(config.blob[0], BLOB_BASE_VALUE); 
  uint16_t index = TEST_DATA_LENGTH-1;
  assertEqual(config.blob[index], BLOB_BASE_VALUE + index); 
  
  config.blob[index] = BLOB_NEW_VALUE;
  config.save();
  
  TestConfigParams config2 = TestConfigParams();
  config2.load();
  assertEqual(config2.version(), TEST_DATA_VERSION);
  assertEqual(config2.blob[0], BLOB_BASE_VALUE);
  assertEqual(config2.blob[index], BLOB_NEW_VALUE);
  
  config2.clear();
  assertEqual(config2.version(), 0L);
  assertEqual(config2.blob[0], 0);
  assertEqual(config2.blob[index], 0);
}

