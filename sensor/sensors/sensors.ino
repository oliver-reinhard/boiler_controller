#line 2 "sensors.ino"
#include "ArduinoUnit.h"

#define MOCK_ONE_WIRE true

#include "OneWireSensors.h"

//#define DEBUG_SENSORS_MAIN

#define ONE_WIRE_PIN 10

#if defined MOCK_ONE_WIRE
  MockOneWire oneWire = MockOneWire(ONE_WIRE_PIN);
#else
  OneWire oneWire = OneWire(ONE_WIRE_PIN);  // on pin 10 (a 4.7K pull-up resistor to +5V is necessary)
#endif

uint8_t SENSOR_IDS[][TEMP_SENSOR_ID_BYTES] = { {0x28, 0x8C, 0x8C, 0x79, 0x06, 0x00, 0x00, 0x89}, 
                                               {0x28, 0x7C, 0x28, 0x79, 0x06, 0x00, 0x00, 0xD7} };

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

test(sensor_setup) {
  DS18B20TemperatureSensor s1 = DS18B20TemperatureSensor("Sensor 1");
  s1.rangeMin = -1000;
  s1.rangeMax = 10000;
  
  assertEqual(s1.sensorStatus, SENSOR_INITIALISING);
  
  DS18B20TemperatureSensor s2 = DS18B20TemperatureSensor("Sensor 2");
  DS18B20TemperatureSensor s3 = DS18B20TemperatureSensor("Sensor 3");
  
  DS18B20TemperatureSensor *sensors[] = {&s1, &s2, &s3};

  DS18B20Controller controller = DS18B20Controller(&oneWire, sensors, 3);

  #ifdef MOCK_ONE_WIRE
    oneWire._setSearchResults((uint8_t *) SENSOR_IDS, 2);
  #endif
  
  controller.setupSensors();
  assertEqual(s1.sensorStatus, SENSOR_ID_AUTO_ASSIGNED);
  
  assertEqual(s2.sensorStatus, SENSOR_ID_AUTO_ASSIGNED);
  assertEqual(s3.sensorStatus, SENSOR_ID_UNDEFINED);
  #ifdef MOCK_ONE_WIRE
    assertTrue(! memcmp(s1.id, SENSOR_IDS[0], TEMP_SENSOR_ID_BYTES));
    assertTrue(! memcmp(s2.id, SENSOR_IDS[1], TEMP_SENSOR_ID_BYTES));
  #else
    assertTrue(! DS18B20Controller::isUndefinedTempSensorID(s1.id));
    assertTrue(! DS18B20Controller::isUndefinedTempSensorID(s2.id));
  #endif
  assertTrue(DS18B20Controller::isUndefinedTempSensorID(s3.id));
}

