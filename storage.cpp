#include "storage.h"
#include "message.h"
#include "state.h"
#include "config.h"
#include "log.h"
#include "unit_test.h"
#include <assert.h>
#include <EEPROM.h>

#define DEBUG_STORAGE

const unsigned short VERSION_SIZE = sizeof(Version);
const unsigned short CONFIG_SIZE = sizeof(ConfigParams);
const unsigned short LOG_ENTRY_SIZE = sizeof(LogEntry);

const unsigned short EEPROM_VERSION_OFFSET = 0;
const unsigned short EEPROM_CONFIG_OFFSET  = EEPROM_VERSION_OFFSET + VERSION_SIZE;
const unsigned short EEPROM_LOG_OFFSET     = EEPROM_CONFIG_OFFSET + CONFIG_SIZE;

#ifndef UNIT_TEST
const unsigned short STORAGE_SIZE = EEPROM.length();
#endif
#ifdef UNIT_TEST
const unsigned short STORAGE_SIZE = EEPROM_LOG_OFFSET + UNIT_TEST_LOG_ENTRIES * LOG_ENTRY_SIZE;
#endif
const unsigned short LOG_ENTRIES = (STORAGE_SIZE - EEPROM_LOG_OFFSET) / LOG_ENTRY_SIZE;

/*
 * GLOBAL VARIABLES
 */
unsigned short logHead = 0; // index of the next empty log entry (yet to be written); the entry pointed to has been cleared already
unsigned short logTail = 0; // index of the oldest log entry (there is always one!)

/*
 * AUXILIARY FUNCTIONS
 */

unsigned short eepromLogOffset(unsigned short index) {
  return EEPROM_LOG_OFFSET + index * LOG_ENTRY_SIZE;
}

/**
 * Clears the log entry at the current index but does not update logHead or logTail.
 */
void clearLogEntry(unsigned short index) {
  unsigned short offset = eepromLogOffset(index);
  for (unsigned short i = 0; i < LOG_ENTRY_SIZE ; i++) {
    EEPROM.update(offset + i, 0);
  }
}

/**
 * Writes a log entry at the current logHead position and updates logHead and logTail.
 */
void writeLogEntry(const LogEntry *entry) {
  EEPROM.put(eepromLogOffset(logHead), *entry);
  logHead = (logHead + 1) % LOG_ENTRIES;
  if (logHead == logTail) {
    logTail = (logTail + 1) % LOG_ENTRIES;
  }
  // clear the next entry
  clearLogEntry(logHead);
}

/*
 * EXPORTED FUNCTIONS
 */

Version version() {
  Version v;
  EEPROM.get(EEPROM_VERSION_OFFSET, v);
  return v;
}

unsigned short size() {
  return STORAGE_SIZE;
}

unsigned short maxLogEntries() {
  return LOG_ENTRIES;
}

unsigned short currentLogEntries() {
  if (logHead == logTail) return 0;
  if (logHead > logTail) return logHead - logTail;
  return LOG_ENTRIES - (logTail - logHead);
}

/**
 * Exported only for unit testing.
 */
void readConfigParams(ConfigParams *configParams) {
  EEPROM.get(EEPROM_CONFIG_OFFSET, *configParams);
}

/**
 * Exported only for unit testing.
 */
void initConfigParams(ConfigParams *configParams, boolean *updated) {
  *updated = false;
  
  if (configParams->targetTemp == 0) {
    configParams->targetTemp = DEFAULT_TARGET_TEMP;
    *updated = true;
  }

  if (configParams->waterSensorCutOutTemp == 0) {
    configParams->waterSensorCutOutTemp = DEFAULT_WATER_SENSOR_CUT_OUT_TEMP;
    *updated = true;
  }

  if (configParams->waterSensorBackOkTemp == 0) {
    configParams->waterSensorBackOkTemp = DEFAULT_WATER_SENSOR_BACK_OK_TEMP;
    *updated = true;
  }

  if (configParams->logTempDelta == 0) {
    configParams->logTempDelta = DEFAULT_LOG_TEMP_DELTA;
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

void getConfigParams(ConfigParams *configParams) {
  if (version() != EEPROM_LAYOUT_VERSION) {
    #ifdef DEBUG_STORAGE
    Serial.println("Updating EEPROM Layout Version");
    #endif
    EEPROM.put(EEPROM_VERSION_OFFSET, EEPROM_LAYOUT_VERSION);
  }

  readConfigParams(configParams);
  boolean updated;
  initConfigParams(configParams, &updated);

  // End initialise
  if (updated) {
    #ifdef DEBUG_STORAGE
    Serial.println("Updating EEPROM ConfigParams");
    #endif
    updateConfigParams(configParams);
  }
}

void updateConfigParams(const ConfigParams *configParams) {
    EEPROM.put(EEPROM_CONFIG_OFFSET, *configParams);
}

void clearConfigParams() {
  // clear version number and configParams block.
  for (unsigned short i = EEPROM_VERSION_OFFSET ; i < EEPROM_CONFIG_OFFSET + CONFIG_SIZE ; i++) {
    EEPROM.write(i, 0);
  }
}

void initLog() {
  LogEntry entry;
  Timestamp latest = 0L;  // timestamp of the the most recent log entry
  logHead = LOG_ENTRIES; // out of range => assert later that logHead was updated!
  // find log head (= first empty log entry)
  for (unsigned short i = 0; i < LOG_ENTRIES ; i++) {
    EEPROM.get(eepromLogOffset(i), entry);
    
    if (entry.timestamp == 0L) {
      logHead = i;
      if(i == 0) {
        // if the head (= empty entry) is the very first entry  of the array, then the most recent is the very last one:
        LogEntry mostRecent;
        EEPROM.get(eepromLogOffset(LOG_ENTRIES-1), mostRecent);      
        latest = mostRecent.timestamp;
      }
      break;
    } 
    latest = entry.timestamp;
  }
  
  assert(logHead != LOG_ENTRIES);
  assert(latest != 0L);
  adjustLogTime(latest);
  
  logTail = LOG_ENTRIES; // out of range => assert later that logHead was updated!
  for (unsigned short i = 1; i < LOG_ENTRIES ; i++) {
    unsigned short index = (logHead + i) % LOG_ENTRIES;
    EEPROM.get(eepromLogOffset(index), entry);
    if (entry.timestamp != 0L) {
      logTail = index;
      break;
    }
  }
  assert(logTail != LOG_ENTRIES);
}

void resetLog() {
  // clear
  for (unsigned short i = 0; i < LOG_ENTRIES; i++) {
    clearLogEntry(i);
  }
  resetLogTime();
  logHead = 0;
  logTail = 0;
  // write a log message so there is always at least one log entry:
  logMessage(MSG_LOG_INIT, 0, 0);
}

void logValues(Temperature water, Temperature ambient, Flags flags) {
  LogEntry entry = createLogValuesEntry(water, ambient, flags);
  writeLogEntry(&entry);
}


void logMessage(MessageID id, short param1, short param2) {
  LogEntry entry = createLogValuesEntry(id, param1, param2);
  writeLogEntry(&entry);
}


/*
 * TESTING ONLY
 */
#ifdef UNIT_TEST

unsigned short logHeadIndex() {
  return logHead;
}
unsigned short logTailIndex() {
  return logTail;
}

#endif


