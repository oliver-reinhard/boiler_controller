#ifndef UT_ONEWIRESENSORS_H_INCLUDED
  #define UT_ONEWIRESENSORS_H_INCLUDED

  // #define DEBUG_MOCK_ONE_WIRE
  
  #define TEMP_SENSOR_ID_BYTES 8
  #define TEMP_SENSOR_READOUT_BYTES 9

  class MockOneWire {
    public:
      MockOneWire(uint8_t) { }
      
      uint8_t reset(void) { return 0; }      
      void select(const uint8_t rom[TEMP_SENSOR_ID_BYTES]) { if(rom == 0) { } }      
      void skip(void) { }
      void write(uint8_t v, uint8_t power = 0) { if (v==0 || power == 0) { } }
  

      String formatTempSensorID(uint8_t *id) {
        char s[3*TEMP_SENSOR_ID_BYTES];
        uint8_t pos = 0;
        for (uint8_t i=0; i<TEMP_SENSOR_ID_BYTES; i++) {
          sprintf(&s[pos], "%02X", id[i]);
          pos += 2;
          if (i < TEMP_SENSOR_ID_BYTES - 1) {
            s[pos++] = '-';
          }
        }
        s[pos++] = '\0';
        return s;
      }
      
      void _setSearchResults(uint8_t *addr, uint8_t len) {
        searchResult = addr;
        for(uint8_t i=0; i<len; i++) {
          #ifdef DEBUG_MOCK_ONE_WIRE
            Serial.print("_setSearchResults: ");
            Serial.println(formatTempSensorID(&searchResult[i * TEMP_SENSOR_ID_BYTES]));
          #endif
        }
        searchResultLen = len;
        searchResultIndex = 0;
      }
      
      uint8_t search(uint8_t *newAddr) { 
        if (searchResultIndex >= searchResultLen) {
          return 0;
        }
        uint8_t *sensorAddr = &searchResult[searchResultIndex * TEMP_SENSOR_ID_BYTES];
        searchResultIndex++;
        memcpy(newAddr, sensorAddr, TEMP_SENSOR_ID_BYTES);
        #ifdef DEBUG_MOCK_ONE_WIRE
          Serial.print("search returns ");
          Serial.println(formatTempSensorID(newAddr));
        #endif
        return 1;
      }
      
      void reset_search() {
        searchResultIndex = 0;
        readIndex = 0;
      }

      void _setReadResults(uint8_t *readResult, uint8_t len) {
        this->readResult = readResult;
        readResultLen = len;
        readIndex = 0;
      }
      
      uint8_t read(void) { 
        if (readIndex >= readResultLen) {
          return 0;
        }
        return readResult[readIndex++];
      }

    private:
      uint8_t *searchResult;
      uint8_t searchResultLen = 0;
      uint8_t searchResultIndex = 0;
      uint8_t *readResult;
      uint8_t readResultLen = 0;
      uint8_t readIndex = 0;
  };
  
#endif
