#include "control.h"
#include "msg.h"

// #define DEBUG_CONTROL

/*
 * Digital Temperature Sensor DS18B20 commands (see sensor data sheet)
 */
const uint8_t CMD_CONVERT_TEMP     = 0x44;  // write(0x44, 1)  // 1 = keep line high during conversion
const uint8_t CMD_READ_SCRATCHPAD  = 0xBE;  // write(0xBE)

/*
 * Number of bytes of data vector returned by the DS18B20: 8 byte data + 1 byte CRC
 */
const uint8_t TEMP_SENSOR_READOUT_BYTES = 9;

/*
 * Configuration-byte position in data vector returned by the DS18B20
 */
const uint8_t DATA_CONFIG_BYTE = 4;

uint32_t heatingTotalMillis(OperationalParams *op) {
  uint32_t duration = op->heatingAccumulatedMillis;
  if (op->heatingStartMillis != 0L) {
    duration += millis() - op->heatingStartMillis;
  }
  return duration;
}

boolean undefinedSensorId(TempSensorID addr) {
  return ! memcmp(addr, UNDEFINED_SENSOR_ID, TEMP_SENSOR_ID_BYTES);
}

boolean isWaterSensor(TempSensorID addr, ConfigParams *config) {
  return ! memcmp(addr, config->waterTempSensorId, TEMP_SENSOR_ID_BYTES);
}

boolean isAmbientSensor(TempSensorID addr, ConfigParams *config) {
  return ! memcmp(addr, config->ambientTempSensorId, TEMP_SENSOR_ID_BYTES);
}


/*
 * @param addr  4-byte ID of DS18B20 temperature sensor
 * @param data  data vector returned by the DS18B20: 8 byte data + 1 byte CRC
 */
boolean ControlActions::readScratchpad(uint8_t addr[], uint8_t *data) {
  oneWire.reset();
  // Talk only to sensor with 'addr':
  oneWire.select(addr);
  oneWire.write(CMD_READ_SCRATCHPAD);
  
  // Read 8 byte of data + 1 byte of CRC
  for (uint8_t i = 0; i < TEMP_SENSOR_READOUT_BYTES; i++) {           
    data[i] = oneWire.read();
  }
  oneWire.reset();
  return OneWire::crc8(data, TEMP_SENSOR_READOUT_BYTES-1) == data[TEMP_SENSOR_READOUT_BYTES-1];
}

/*
 * Parse temperature [°C] from raw DS18B20 data
 * @param data  data vector returned by the DS18B20: 8 byte data + 1 byte CRC
 */
TemperatureReadout ControlActions::getCelcius(uint8_t data[]) {
  int16_t raw = (data[1] << 8) | data[0];
  uint8_t config = (data[DATA_CONFIG_BYTE] & 0x60);
  
  TemperatureReadout temp;
  // At lower resolution, the low bits are undefined, so let's zero them
  if (config == 0x00) {
    raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    temp.resolution = 9;
  } else if (config == 0x20) {
    raw = raw & ~3; // 10 bit res, 187.5 ms
    temp.resolution = 10;
  } else if (config == 0x40) {
    raw = raw & ~1; // 11 bit res, 375 ms
    temp.resolution = 11;
  } else {
    temp.resolution = 12;
    // default is 12 bit resolution, 750 ms conversion time
  }
  temp.celcius = (int16_t)(raw * 100.0 / 16.0);  // [°C * 100]
  return temp;
}

void ControlActions::setupSensors(ControlContext *context) {
  if (undefinedSensorId(context->config->waterTempSensorId)) {
   context->op->water.sensorStatus = SENSOR_ID_UNDEFINED;
   context->storage->logMessage(MSG_WATER_TEMP_SENSOR_ID_UNDEF, 0, 0); 
   #ifdef DEBUG_CONTROL
     Serial.println(F("DEBUG_CONTROL: Water temp sensor ID undefined"));
   #endif
  }
  
  if (undefinedSensorId(context->config->ambientTempSensorId)) {
   context->op->ambient.sensorStatus = SENSOR_ID_UNDEFINED;
   context->storage->logMessage(MSG_AMBIENT_TEMP_SENSOR_ID_UNDEF, 0, 0);
   #ifdef DEBUG_CONTROL
     Serial.println(F("DEBUG_CONTROL: Ambient temp sensor ID undefined"));
   #endif
  } 

  //
  // CONSIDER: set sensor resolution
  // CONSIDER: detect sensors and store IDs for user to choose which sensor is which
}

void ControlActions::initSensorReadout(ControlContext *context) {
  if (context == NULL) { } // prevent warning "unused parameter ..."
  oneWire.reset();
  // Talk to all slaves on bus:
  oneWire.skip();
  // Start temp readout and conversion to scratchpad, with parasite power on at the end
  oneWire.write(CMD_CONVERT_TEMP, 1);
  //
  // Important: do not oneWire.reset() now for at least 750 ms (for 12-bit temperature resolution)
  //
}


void ControlActions::completeSensorReadout(ControlContext *context) {
  context->op->water.sensorStatus   = SENSOR_NOK;
  context->op->water.currentTemp    = UNDEFINED_TEMPERATURE;
  context->op->ambient.sensorStatus = SENSOR_NOK;
  context->op->ambient.currentTemp  = UNDEFINED_TEMPERATURE;
  
  uint8_t addr[TEMP_SENSOR_ID_BYTES];
  
  while(oneWire.search(addr)) {  
    if (OneWire::crc8(addr, TEMP_SENSOR_ID_BYTES-1) != addr[TEMP_SENSOR_ID_BYTES-1]) {
      continue;
    }
    
    uint8_t data[TEMP_SENSOR_READOUT_BYTES];
    if (! readScratchpad(addr, &data[0])) {
      continue;
    }
    TemperatureReadout readout = getCelcius(data);
    
    if (isWaterSensor(addr, context->config)) {
      // Ensure temperature is plausible:
      if( readout.celcius >= WATER_MIN_TEMP && readout.celcius <= WATER_MAX_TEMP) {
         context->op->water.sensorStatus = SENSOR_OK;
         context->op->water.currentTemp  = readout.celcius;
         #ifdef DEBUG_CONTROL
           Serial.print(F("DEBUG_CONTROL: Water: Sensor OK, temp: "));
           Serial.println(readout.celcius);
         #endif
      } 
      #ifdef DEBUG_CONTROL
        else {
          Serial.println(F("DEBUG_CONTROL: Water: Sensor NOK"));
        }
      #endif
    } else if (isAmbientSensor(addr, context->config)) {
      // Ensure temperature is plausible:
      if( readout.celcius >= AMBIENT_MIN_TEMP && readout.celcius <= AMBIENT_MAX_TEMP) {
         context->op->ambient.sensorStatus = SENSOR_OK;
         context->op->ambient.currentTemp  = readout.celcius;
         #ifdef DEBUG_CONTROL
           Serial.print("DEBUG_CONTROL: Ambient: Sensor OK, temp: "));
           Serial.println(readout.celcius);
         #endif
      }
      #ifdef DEBUG_CONTROL
        else {
          Serial.println(F("DEBUG_CONTROL: Ambient: Sensor NOK"));
        }
      #endif
    }
  }
  oneWire.reset_search();
}

void ControlActions::heat(boolean on, ControlContext *context) {
  context->op->heating = on;
  if (on) {
    digitalWrite(HEATER_PIN, HIGH);
  } else {
    digitalWrite(HEATER_PIN, LOW);
  }
}


void ControlActions::setConfigParam() {
  pendingRequests |= WRITE_CONFIG;
}

void ControlActions::requestHelp() {
  pendingRequests |= READ_HELP;
}

void ControlActions::requestLog() {
  pendingRequests |= READ_LOG;
}

void ControlActions::requestConfig() {
  pendingRequests |= READ_CONFIG;
}

void ControlActions::requestStat() {
  pendingRequests |= READ_STAT;
}

ReadWriteRequests ControlActions::getPendingReadWriteRequests() {
  return pendingRequests;
}

void ControlActions::clearPendingReadWriteRequests() {
  pendingRequests = READ_WRITE_NONE;
}

