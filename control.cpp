#include "control.h"

// #define DEBUG_CONTROL


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



void ControlActions::setupSensors() {
  if (undefinedSensorId(context->config->waterTempSensorId)) {
   context->op->water.sensorStatus = SENSOR_ID_UNDEFINED;
   context->log->logMessage(MSG_WATER_TEMP_SENSOR_ID_UNDEF, 0, 0); 
   #ifdef DEBUG_CONTROL
     Serial.println(F("DEBUG_CONTROL: Water temp sensor ID undefined"));
   #endif
  }
  
  if (undefinedSensorId(context->config->ambientTempSensorId)) {
   context->op->ambient.sensorStatus = SENSOR_ID_UNDEFINED;
   context->log->logMessage(MSG_AMBIENT_TEMP_SENSOR_ID_UNDEF, 0, 0);
   #ifdef DEBUG_CONTROL
     Serial.println(F("DEBUG_CONTROL: Ambient temp sensor ID undefined"));
   #endif
  } 

  //
  // CONSIDER: set sensor resolution
  // CONSIDER: detect sensors and store IDs for user to choose which sensor is which
}

void ControlActions::initSensorReadout() {
  oneWire.reset();
  // Talk to all slaves on bus:
  oneWire.skip();
  // Start temp readout and conversion to scratchpad, with parasite power on at the end
  oneWire.write(CMD_CONVERT_TEMP, 1);
  //
  // Important: do not oneWire.reset() now for at least 750 ms (for 12-bit temperature resolution)
  //
}


void ControlActions::completeSensorReadout() {
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

void ControlActions::heat(boolean on) {
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

