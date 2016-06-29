#include "Logging.h"

// #define DEBUG_LOG

#define LOG_ENTRY_SIZE sizeof(LogEntry)
#define NUM_SLOTS_SIZE sizeof(uint16_t)
#define ASSERT(cond, msg) ((cond) ? (void)0 : S_O_S(F(msg)))


AbstractLog::AbstractLog(const uint16_t eepromOffset, const uint16_t eepromlogSize) {
  this->eepromOffset = eepromOffset;
  this->eepromLogSize = eepromlogSize;
  logEntrySlots = (eepromLogSize - NUM_SLOTS_SIZE) / LOG_ENTRY_SIZE;
}   

uint16_t AbstractLog::entryOffset(uint16_t index) {
  return eepromOffset + NUM_SLOTS_SIZE + index * LOG_ENTRY_SIZE;
}

uint16_t AbstractLog::maxLogEntries() {
  return logEntrySlots - 1;
}

uint16_t AbstractLog::currentLogEntries() {
  if (logHeadIndex == logTailIndex) return 0;
  if (logHeadIndex > logTailIndex) return logHeadIndex - logTailIndex;
  return logEntrySlots - (logTailIndex - logHeadIndex);  // (logTailIndex - logHeadIndex) always differs by at least 1
}

void AbstractLog::clear() {
  #ifdef DEBUG_LOG
    Serial.print(F("DEBUG_LOG: resetLog(), [new] log size = "));
    Serial.println(logEntrySlots);
  #endif
  EEPROM.put(eepromOffset, logEntrySlots);
  // clear
  for (uint16_t i = 0; i < logEntrySlots; i++) {
    clearLogEntry(i);
  }
  logTime.reset();
  logHeadIndex = 0;
  logTailIndex = 0;
  // write a log message so there is always at least one log entry:
  logMessage(MSG_LOG_INIT, 0, 0);
  lastNotifiedLogEntryIndex = logEntrySlots - 1; // = the one before the current entry at index 0
}


void AbstractLog::init() {
  //
  // Check if the number of log entries has changed (typically by changing from unit tests to production):
  //
  uint16_t oldMaxLogEntries;
  EEPROM.get(eepromOffset, oldMaxLogEntries);
  #ifdef DEBUG_LOG
    Serial.print(F("DEBUG_LOG: initlog(), stored log size = "));
    Serial.println(oldMaxLogEntries);
  #endif
  if (oldMaxLogEntries != logEntrySlots) {
    clear();
    logMessage(MSG_LOG_SIZE_CHG, oldMaxLogEntries, logEntrySlots);
    return;
  }
  
  LogEntry entry;
  uint16_t mostRecentIndex = logEntrySlots;  // index of the most recent log entry (value is out of range => assert later that it was updated!)
  Timestamp mostRecentTimestamp = 0L;        // timestamp of the  most recent log entry
  logHeadIndex = logEntrySlots;              // value is out of range => assert later that logHeadIndex was updated!
  
  // find log head (= first empty log entry)
  for (uint16_t i = 0; i < logEntrySlots ; i++) {
    EEPROM.get(entryOffset(i), entry);
    
    if (entry.timestamp == 0L) {
      logHeadIndex = i;
      mostRecentIndex = (logEntrySlots + logHeadIndex - 1) % logEntrySlots;  // (logHeadIndex -1) can be negative => % function returns 0 ... !! => ensure always >= 0
      if(i == 0) {
        // if the head (= empty entry) is the very first entry of the array, then the most recent one is the very last one:
        LogEntry mostRecentEntry;
        EEPROM.get(entryOffset(mostRecentIndex), mostRecentEntry);      
        mostRecentTimestamp = mostRecentEntry.timestamp;
      }
      break;
    } 
    mostRecentTimestamp = entry.timestamp;
  }
  
  ASSERT(mostRecentIndex != logEntrySlots, "initLog:index");
  lastNotifiedLogEntryIndex = mostRecentIndex;
  
  ASSERT(logHeadIndex != logEntrySlots, "initLog:head");
  
  ASSERT(mostRecentTimestamp != 0L, "initLog:timestamp");
  logTime.adjust(mostRecentTimestamp);
  
  logTailIndex = logEntrySlots;    // value is out of range => assert later that logTailIndex was updated!
  for (uint16_t i = 1; i < logEntrySlots ; i++) {
    uint16_t index = (logHeadIndex + i) % logEntrySlots;
    EEPROM.get(entryOffset(index), entry);
    if (entry.timestamp != 0L) {
      logTailIndex = index;
      break;
    }
  }
  ASSERT(logTailIndex != logEntrySlots , "initLog:tail");
}

void AbstractLog::clearLogEntry(uint16_t index) {
  reader.valid = false;
  uint16_t offset = entryOffset(index);
  for (uint16_t i = 0; i < LOG_ENTRY_SIZE ; i++) {
    EEPROM.update(offset + i, 0);
  }
}


/*
 * Generic log-entry creation.
 */
LogEntry AbstractLog::addLogEntry(LogDataTypeID type, LogData *data) {
  LogEntry entry;
  entry.timestamp = logTime.timestamp();
  entry.type = type;
  #ifdef DEBUG_LOG
    Serial.print(F("DEBUG_LOG: createLogEntry: type "));
    Serial.print(entry.type);
    Serial.print(F(" => timestamp: "));
    Serial.println(logTime.format(entry.timestamp));
  #endif
  memcpy(&(entry.data), data, sizeof(LogData));
  
  EEPROM.put(entryOffset(logHeadIndex), entry);
  logHeadIndex = (logHeadIndex + 1) % logEntrySlots;
  if (logHeadIndex == logTailIndex) {
    logTailIndex = (logTailIndex + 1) % logEntrySlots;
  }
  // clear the next entry
  clearLogEntry(logHeadIndex);
  
  return entry;
}

void AbstractLog::readMostRecentLogEntries(uint16_t maxResults) {
  reader.kind = LOG_READER_MOST_RECENT;
  uint16_t n = currentLogEntries();
  if (maxResults == 0) {
      reader.toRead = n;
  } else {
    reader.toRead = maxResults < n ? maxResults : n;
  }
  reader.read = 0;
  reader.nextIndex = (logEntrySlots + logHeadIndex - 1) % logEntrySlots; // (logHeadIndex -1) can be negative => % function returns 0 ... !! => ensure always >= 0
  #ifdef DEBUG_LOG
    Serial.print(F("DEBUG_LOG: readMostRecentLogEntries: nextIndex "));
    Serial.println(reader.nextIndex);
  #endif
  reader.valid = true;
}


void AbstractLog::readUnnotifiedLogEntries() {
  reader.kind = LOG_READER_UNNOTIFIED;
  if (logHeadIndex > lastNotifiedLogEntryIndex) {
    reader.toRead = logHeadIndex - lastNotifiedLogEntryIndex - 1;
  } else {
    reader.toRead = logEntrySlots - (lastNotifiedLogEntryIndex - logHeadIndex) - 1;
  }
  reader.read = 0;
  reader.nextIndex = (lastNotifiedLogEntryIndex + 1) % logEntrySlots;
  #ifdef DEBUG_LOG
    Serial.print(F("DEBUG_LOG: readUnnotifiedLogEntries: nextIndex "));
    Serial.println(reader.nextIndex);
  #endif
  reader.valid = true;
}

 
boolean AbstractLog::nextLogEntry(LogEntry &entry) {
  if (reader.valid && reader.read < reader.toRead) {
    EEPROM.get(entryOffset(reader.nextIndex), entry); 
    #ifdef DEBUG_LOG
      Serial.print(F("DEBUG_LOG: getLogEntry: timestamp: "));
      Serial.print(logTime.format(entry->timestamp));
      Serial.print(F(", type: "));
      Serial.println(entry->type);
    #endif
    reader.read++;
    if (reader.kind == LOG_READER_MOST_RECENT) {
      reader.nextIndex = (reader.nextIndex - 1) % logEntrySlots;
      
    } else if (reader.kind == LOG_READER_UNNOTIFIED) {
      lastNotifiedLogEntryIndex = reader.nextIndex;
      reader.nextIndex = (reader.nextIndex + 1) % logEntrySlots;
    }
    #ifdef DEBUG_LOG
      Serial.print(F("DEBUG_LOG: nextLogEntry: nextIndex "));
      Serial.println(reader.nextIndex);
    #endif
    return true;
  }
  return false;
}


void AbstractLog::S_O_S(MessageID id, int16_t param1, int16_t param2) {
  Timestamp ts = logMessage(id, param1, param2);
  ts = ts; // prevents warning: unused variable
  #ifdef DEBUG_LOG
    Serial.print(F("DEBUG_LOG: S.O.S. : See log message "));
    Serial.println(logTime.format(ts));
  #endif
  S_O_S(NULL);
}


void AbstractLog::S_O_S(const __FlashStringHelper *debug) {
  if (debug != NULL) {  // also prevents warning: unused parameter
    #ifdef DEBUG_LOG
      Serial.print(F("DEBUG_LOG: S.O.S. : "));
      Serial.println(debug);
    #endif
  }
  
  int pulse = 300; // [ms]
  while(1) {
    // S.O.S. . . . – – – . . .
    pulse = (pulse + 200) % 400; // toggles between 100 and 300 ms
    delay(pulse);
    for(byte i=0; i<3; i++) {
      digitalWrite(SOS_LED_PIN, HIGH);
      delay(pulse);
      digitalWrite(SOS_LED_PIN, LOW);
      delay(pulse); 
    }      
  }
}


