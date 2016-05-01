#include "storage.h"
#include "message.h"
#include "state.h"
#include "config.h"
#include "log.h"
#include "unit_test.h"
#include <assert.h>
#include <EEPROM.h>

//#define DEBUG_STORAGE

const unsigned short VERSION_SIZE = sizeof(Version);
const unsigned short CONFIG_SIZE = sizeof(ConfigParams);
const unsigned short LOG_ENTRY_SIZE = sizeof(LogEntry);

const unsigned short EEPROM_VERSION_OFFSET     = 0;
const unsigned short EEPROM_CONFIG_OFFSET      = EEPROM_VERSION_OFFSET + VERSION_SIZE;
const unsigned short EEPROM_LOG_OFFSET         = EEPROM_CONFIG_OFFSET + CONFIG_SIZE;
const unsigned short EEPROM_LOG_ENTRIES_OFFSET = EEPROM_LOG_OFFSET + sizeof(unsigned short);

#ifndef UNIT_TEST
  const unsigned short STORAGE_SIZE = EEPROM.length();
#endif
#ifdef UNIT_TEST
  const unsigned short STORAGE_SIZE = EEPROM_LOG_ENTRIES_OFFSET + UNIT_TEST_LOG_ENTRIES * LOG_ENTRY_SIZE;
#endif
const unsigned short MAX_LOG_ENTRIES = (STORAGE_SIZE - EEPROM_LOG_ENTRIES_OFFSET) / LOG_ENTRY_SIZE;

/*
 * AUXILIARY FUNCTIONS
 */
unsigned short eepromLogOffset(unsigned short index) {
  return EEPROM_LOG_ENTRIES_OFFSET + index * LOG_ENTRY_SIZE;
}

/*
 * EXPORTED FUNCTIONS
 */

Version Storage::version() {
  Version v;
  EEPROM.get(EEPROM_VERSION_OFFSET, v);
  return v;
}

unsigned short Storage::size() {
  return STORAGE_SIZE;
}

/*
 * CONFIG PARAMS
 */
void Storage::clearConfigParams() {
  // clear version number and configParams block.
  for (unsigned short i = EEPROM_VERSION_OFFSET ; i < EEPROM_CONFIG_OFFSET + CONFIG_SIZE ; i++) {
    EEPROM.write(i, 0);
  }
}

void Storage::getConfigParams(ConfigParams *configParams) {
  if (version() != EEPROM_LAYOUT_VERSION) {
    #ifdef DEBUG_STORAGE
      Serial.println("DEBUG_STORAGE: Updating EEPROM Layout Version");
    #endif
    EEPROM.put(EEPROM_VERSION_OFFSET, EEPROM_LAYOUT_VERSION);
  }

  readConfigParams(configParams);
  boolean updated;
  initConfigParams(configParams, &updated);

  // End initialise
  if (updated) {
    #ifdef DEBUG_STORAGE
      Serial.println("DEBUG_STORAGE: Updating EEPROM ConfigParams");
    #endif
    updateConfigParams(configParams);
  }
}

void Storage::updateConfigParams(const ConfigParams *configParams) {
    EEPROM.put(EEPROM_CONFIG_OFFSET, *configParams);
}

void Storage::readConfigParams(ConfigParams *configParams) {
  EEPROM.get(EEPROM_CONFIG_OFFSET, *configParams);
}

void Storage::initConfigParams(ConfigParams *configParams, boolean *updated) {
  *updated = false;
  
  if (configParams->targetTemp == 0) {
    configParams->targetTemp = DEFAULT_TARGET_TEMP;
    *updated = true;
  }

  // Temp Sensor IDs do not have a default value => need to be set as part of the installation .

  if (configParams->heaterCutOutWaterTemp == 0) {
    configParams->heaterCutOutWaterTemp = DEFAULT_HEATER_CUT_OUT_WATER_TEMP;
    *updated = true;
  }

  if (configParams->heaterBackOkWaterTemp == 0) {
    configParams->heaterBackOkWaterTemp = DEFAULT_HEATER_BACK_OK_WATER_TEMP;
    *updated = true;
  }

  if (configParams->logTempDelta == 0) {
    configParams->logTempDelta = DEFAULT_LOG_TEMP_DELTA;
    *updated = true;
  }

  if (configParams->logTimeDelta == 0L) {
    configParams->logTimeDelta = DEFAULT_LOG_TIME_DELTA;
    *updated = true;
  }
  
  if (configParams->tankCapacity == 0.0) {
    configParams->tankCapacity = DEFAULT_TANK_CAPACITY;
    *updated = true;
  }

  if (configParams->heaterPower == 0.0) {
    configParams->heaterPower = DEFAULT_HEATER_POWER;
    *updated = true;
  }

  if (configParams->insulationFactor == 0.0) {
    configParams->insulationFactor = DEFAULT_INSULATION_FACTOR;
    *updated = true;
  }

  // *** Initialise new config parameters here:
}

/*
 * LOGGING
 */

unsigned short Storage::maxLogEntries() {
  return MAX_LOG_ENTRIES;
}

unsigned short Storage::currentLogEntries() {
  if (logHead == logTail) return 0;
  if (logHead > logTail) return logHead - logTail;
  return MAX_LOG_ENTRIES - (logTail - logHead);
}

void Storage::resetLog() {
  #ifdef DEBUG_STORAGE
    Serial.print("DEBUG_STORAGE: resetLog(), [new] log size = ");
    Serial.println(MAX_LOG_ENTRIES);
  #endif
  EEPROM.put(EEPROM_LOG_OFFSET, MAX_LOG_ENTRIES);
  // clear
  for (unsigned short i = 0; i < MAX_LOG_ENTRIES; i++) {
    clearLogEntry(i);
  }
  resetLogTime();
  logHead = 0;
  logTail = 0;
  // write a log message so there is always at least one log entry:
  logMessage(MSG_LOG_INIT, 0, 0);
}

void Storage::initLog() {
  //
  // Check if the number of log entries has changed (typically by changing from unit tests to production):
  //
  unsigned short oldMaxLogEntries;
  EEPROM.get(EEPROM_LOG_OFFSET, oldMaxLogEntries);
  #ifdef DEBUG_STORAGE
    Serial.print("DEBUG_STORAGE: initlog(), stored log size = ");
    Serial.println(oldMaxLogEntries);
  #endif
  if (oldMaxLogEntries != MAX_LOG_ENTRIES) {
    resetLog();
    logMessage(MSG_LOG_SIZE_CHG, oldMaxLogEntries, MAX_LOG_ENTRIES);
    return;
  }
  
  LogEntry entry;
  Timestamp latest = 0L;  // timestamp of the the most recent log entry
  logHead = MAX_LOG_ENTRIES; // out of range => assert later that logHead was updated!
  // find log head (= first empty log entry)
  for (unsigned short i = 0; i < MAX_LOG_ENTRIES ; i++) {
    EEPROM.get(eepromLogOffset(i), entry);
    
    if (entry.timestamp == 0L) {
      logHead = i;
      if(i == 0) {
        // if the head (= empty entry) is the very first entry  of the array, then the most recent is the very last one:
        LogEntry mostRecent;
        EEPROM.get(eepromLogOffset(MAX_LOG_ENTRIES-1), mostRecent);      
        latest = mostRecent.timestamp;
      }
      break;
    } 
    latest = entry.timestamp;
  }
  
  assert(logHead != MAX_LOG_ENTRIES);
  assert(latest != 0L);
  adjustLogTime(latest);
  
  logTail = MAX_LOG_ENTRIES; // out of range => assert later that logHead was updated!
  for (unsigned short i = 1; i < MAX_LOG_ENTRIES ; i++) {
    unsigned short index = (logHead + i) % MAX_LOG_ENTRIES;
    EEPROM.get(eepromLogOffset(index), entry);
    if (entry.timestamp != 0L) {
      logTail = index;
      break;
    }
  }
  assert(logTail != MAX_LOG_ENTRIES);
}

Timestamp Storage::logValues(Temperature water, Temperature ambient, Flags flags) {
  LogEntry entry = createLogValuesEntry(water, ambient, flags);
  writeLogEntry(&entry);
  return entry.timestamp;
}

Timestamp Storage::logState(StateID previous, StateID current, EventID event) {
  LogEntry entry = createLogStateEntry(previous, current, event);
  writeLogEntry(&entry);
  return entry.timestamp;
}

Timestamp Storage::logMessage(MessageID id, short param1, short param2) {
  LogEntry entry = createLogValuesEntry(id, param1, param2);
  writeLogEntry(&entry);
  return entry.timestamp;
}


void Storage::clearLogEntry(unsigned short index) {
  unsigned short offset = eepromLogOffset(index);
  for (unsigned short i = 0; i < LOG_ENTRY_SIZE ; i++) {
    EEPROM.update(offset + i, 0);
  }
}

void Storage::writeLogEntry(const LogEntry *entry) {
  #ifdef DEBUG_STORAGE
    Serial.print("DEBUG_STORAGE: Writing log entry, type = ");
    Serial.println(entry->type);
  #endif
  EEPROM.put(eepromLogOffset(logHead), *entry);
  logHead = (logHead + 1) % MAX_LOG_ENTRIES;
  if (logHead == logTail) {
    logTail = (logTail + 1) % MAX_LOG_ENTRIES;
  }
  // clear the next entry
  clearLogEntry(logHead);
}


unsigned short Storage::logHeadIndex() {
  return logHead;
}
unsigned short Storage::logTailIndex() {
  return logTail;
}


