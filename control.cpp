#include "control.h"
#include "message.h"

/*
 * Digital Temperature Sensor DS18B20 commands (see sensor data sheet)
 */
const byte CMD_CONVERT_TEMP     = 0x44;  // write(0x44, 1)  // 1 = keep line high during conversion
const byte CMD_READ_SCRATCHPAD  = 0xBE;  // write(0xBE)

/*
 * Number of bytes of data vector returned by the DS18B20: 8 byte data + 1 byte CRC
 */
const byte TEMP_SENSOR_READOUT_BYTES = 9;

/*
 * Configuration-byte position in data vector returned by the DS18B20
 */
const byte DATA_CONFIG_BYTE = 4;


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
boolean ControlActions::readScratchpad(byte addr[], byte *data) {
  oneWire.reset();
  // Talk only to sensor with 'addr':
  oneWire.select(addr);
  oneWire.write(CMD_READ_SCRATCHPAD);
  
  // Read 8 byte of data + 1 byte of CRC
  for (byte i = 0; i < TEMP_SENSOR_READOUT_BYTES; i++) {           
    data[i] = oneWire.read();
  }
  return OneWire::crc8(data, TEMP_SENSOR_READOUT_BYTES-1) == data[TEMP_SENSOR_READOUT_BYTES-1];
}

/*
 * Parse temperature [°C] from raw DS18B20 data
 * @param data  data vector returned by the DS18B20: 8 byte data + 1 byte CRC
 */
TemperatureReadout ControlActions::getCelcius(byte data[]) {
  int16_t raw = (data[1] << 8) | data[0];
  byte config = (data[DATA_CONFIG_BYTE] & 0x60);
  
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
  temp.celcius = (short)(raw / 16.0) * 100;  // [°C * 100]
  return temp;
}

void ControlActions::setupSensors(ControlContext *context) {
  if (undefinedSensorId(context->config->waterTempSensorId)) {
   context->op->water.sensorStatus = SENSOR_ID_UNDEFINED;
   context->storage->logMessage(MSG_WATER_TEMP_SENSOR_ID_UNDEF, 0, 0);
  }
  
  if (undefinedSensorId(context->config->ambientTempSensorId)) {
   context->op->ambient.sensorStatus = SENSOR_ID_UNDEFINED;
   context->storage->logMessage(MSG_AMBIENT_TEMP_SENSOR_ID_UNDEF, 0, 0);
  } 

  //
  // CONSIDER: set sensor resoluition
  // CONSIDER: detect sensors and store IDs for user to choose which sensor is which
}

void ControlActions::initSensorReadout(ControlContext *context) {
  byte addr[TEMP_SENSOR_ID_BYTES];
  
  while(oneWire.search(addr)) {  
    if (OneWire::crc8(addr, TEMP_SENSOR_ID_BYTES-1) != addr[TEMP_SENSOR_ID_BYTES-1]) {
      continue;
    }
    if (isWaterSensor(addr, context->config) || isAmbientSensor(addr, context->config)) {
      oneWire.reset();
      // Talk only to sensor with 'addr':
      oneWire.select(addr);
      // Start temp readout and conversion to scratchpad, with parasite power on at the end
      oneWire.write(CMD_CONVERT_TEMP, 1);
    }
  }
  oneWire.reset();
  oneWire.reset_search();
}


void ControlActions::completeSensorReadout(ControlContext *context) {
  context->op->water.sensorStatus   = SENSOR_NOK;
  context->op->water.currentTemp    = UNDEFINED_TEMPERATURE;
  context->op->ambient.sensorStatus = SENSOR_NOK;
  context->op->ambient.currentTemp  = UNDEFINED_TEMPERATURE;
  
  byte addr[TEMP_SENSOR_ID_BYTES];
  
  while(oneWire.search(addr)) {  
    if (OneWire::crc8(addr, TEMP_SENSOR_ID_BYTES-1) != addr[TEMP_SENSOR_ID_BYTES-1]) {
      continue;
    }
    
    byte data[TEMP_SENSOR_READOUT_BYTES];
    if (! readScratchpad(addr, &data[0])) {
      continue;
    }
    TemperatureReadout readout = getCelcius(data);
    
    if (isWaterSensor(addr, context->config)) {
      // Ensure temperature is plausible:
      if( readout.celcius >= WATER_MIN_TEMP && readout.celcius <= WATER_MAX_TEMP) {
         context->op->water.sensorStatus = SENSOR_OK;
         context->op->water.currentTemp  = readout.celcius;
      }
    } else if (isAmbientSensor(addr, context->config)) {
      // Ensure temperature is plausible:
      if( readout.celcius >= AMBIENT_MIN_TEMP && readout.celcius <= AMBIENT_MAX_TEMP) {
         context->op->ambient.sensorStatus = SENSOR_OK;
         context->op->ambient.currentTemp  = readout.celcius;
      }
    }
  }
  oneWire.reset();
  oneWire.reset_search();
}

void ControlActions::readUserCommands(ControlContext *context) {
  if (context == NULL) { } // prevent warning "unused parameter ..."
}

void ControlActions::heat(boolean on, ControlContext *context) {
  context->op->heating = on;
  if (on) {
    digitalWrite(HEATER_PIN, HIGH);
  } else {
    digitalWrite(HEATER_PIN, LOW);
  }
}


void ControlActions::logTemperatureValues(ControlContext *context) {
  if (context->op->loggingValues) {
    unsigned long time = millis();

    if (time - context->op->water.lastLoggedTime > context->config->logTimeDelta || time - context->op->ambient.lastLoggedTime > context->config->logTimeDelta) {
      boolean logValuesNow = false;
      Temperature water = UNDEFINED_TEMPERATURE;
      Temperature ambient = UNDEFINED_TEMPERATURE;
      
      if (context->op->water.sensorStatus == SENSOR_OK && abs(context->op->water.currentTemp - context->op->water.lastLoggedTemp) >= context->config->logTempDelta) {
        logValuesNow = true;
        water = context->op->water.currentTemp;
        
      } else if (context->op->water.sensorStatus == SENSOR_NOK && context->op->water.lastLoggedTemp != UNDEFINED_TEMPERATURE) {
        logValuesNow = true;;
      }
      
      if (context->op->ambient.sensorStatus == SENSOR_OK && abs(context->op->ambient.currentTemp - context->op->ambient.lastLoggedTemp) >= context->config->logTempDelta) {
        logValuesNow = true;
        ambient = context->op->ambient.currentTemp;
        
      } else if (context->op->ambient.sensorStatus == SENSOR_NOK && context->op->ambient.lastLoggedTemp != UNDEFINED_TEMPERATURE) {
        logValuesNow = true;
      }
      
      if (logValuesNow) {
        Flags flags = context->op->water.sensorStatus<<4 | context->op->ambient.sensorStatus;
        context->storage->logValues(context->op->water.currentTemp, context->op->ambient.currentTemp, flags);
        context->op->water.lastLoggedTemp = water;
        context->op->water.lastLoggedTime = time;
        context->op->ambient.lastLoggedTemp = ambient;
        context->op->ambient.lastLoggedTime = time;
      }
    }
  }
}

void ControlActions::setConfigParam() {
  // TODO
}

void ControlActions::getLog() {
  
}

void ControlActions::getConfig() {
  // TODO
}

void ControlActions::getStat() {
  Serial.print("State = ");
  Serial.println();
}

