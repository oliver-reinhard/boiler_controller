#ifndef BC_ONEWIRESENSORS_H_INCLUDED
  #define BC_ONEWIRESENSORS_H_INCLUDED
  
//#define MOCK_ONE_WIRE

  #include <Arduino.h>
  #include <OneWire.h>
  #if defined MOCK_ONE_WIRE
    #include "ut_MockOneWire.h"
  #endif
  
  typedef int16_t Temperature;  // [°C * 100]
  #define UNDEFINED_TEMPERATURE INT16_MIN // [°C * 100];

  #define TEMP_SENSOR_ID_BYTES 8
  typedef uint8_t TempSensorID[TEMP_SENSOR_ID_BYTES];
  const TempSensorID UNDEFINED_SENSOR_ID = {0,0,0,0,0,0,0,0};

  boolean isUndefinedTempSensorID(TempSensorID addr);
  
  typedef enum {
    SENSOR_INITIALISING = 0,
    SENSOR_OK = 1,
    SENSOR_NOK = 2,
    SENSOR_ID_UNDEFINED = 3,
    SENSOR_ID_AUTO_ASSIGNED = 4
  } SensorStatusEnum;

  typedef uint8_t SensorStatusID;
  
  class DS18B20TemperatureSensor {
    public:
      DS18B20TemperatureSensor(const char label[10]) {
        strncpy(this->label, label, sizeof(this->label)-1);
        memcpy(id, UNDEFINED_SENSOR_ID, TEMP_SENSOR_ID_BYTES);
      }
      char label[10];
      TempSensorID id;
      Temperature rangeMin = UNDEFINED_TEMPERATURE;
      Temperature rangeMax = UNDEFINED_TEMPERATURE;
      SensorStatusEnum sensorStatus = SENSOR_INITIALISING;
      Temperature currentTemp = UNDEFINED_TEMPERATURE;
      Temperature lastLoggedTemp = UNDEFINED_TEMPERATURE;
      uint32_t lastLoggedTime = 0L;
  };
  
  struct TemperatureReadout {
    uint8_t resolution;     // number of bits (= 9..12)
    Temperature celcius; // [°C * 100]
  };

  class DS18B20Controller {
    public:
      #if defined MOCK_ONE_WIRE
        DS18B20Controller(MockOneWire *oneWire, DS18B20TemperatureSensor **sensors, uint8_t numSensors) {
          Serial.println("MockOneWire");
      #else
        DS18B20Controller(OneWire *oneWire, DS18B20TemperatureSensor **sensors, uint8_t numSensors) {
          Serial.println("OneWire");
      #endif
        this->oneWire = oneWire;
        this->sensors = sensors;
        this->numSensors = numSensors;
      }
      
      void setupSensors();
      void initSensorReadout();
      void completeSensorReadout();
      
      static boolean isUndefinedTempSensorID(TempSensorID addr);
      
    protected:
      #if defined MOCK_ONE_WIRE
        MockOneWire *oneWire;
      #else
        OneWire *oneWire;
      #endif
      DS18B20TemperatureSensor **sensors;
      uint8_t numSensors;

      DS18B20TemperatureSensor *getSensor(uint8_t addr[]);
      DS18B20TemperatureSensor *getNextUndefinedSensor();
      boolean readScratchpad(uint8_t addr[], uint8_t *data);
      TemperatureReadout getCelcius(uint8_t data[]);
  };

  /**
   * Returns the temperature in a dotted notation, 8 chars + terminal '\0': ["-"]dd.ff"°C" (optional "-", d = degrees Celsius (2 digits), f = fraction (1 digit))
   */
  String formatTemperature(Temperature t);

  String formatTempSensorID(TempSensorID id);

#endif
