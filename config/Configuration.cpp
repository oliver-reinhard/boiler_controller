#include "Configuration.h"
#include <EEPROM.h>

//#define DEBUG_CONFIG

/*
 * Classes with virtual methods come with a "superclass" pointer that uses the
 * first two bytes of the in-memory class object:
 */
#define SUPERCLASS_PTR_SIZE sizeof(uint16_t)
/*
 * We keep the EEPROM storage location (as byte offset) in the configuration object:
 */
#define OFFSET_FIELD_SIZE sizeof(uint16_t)
/*
 * This is where the actual configuration data starts (EEPROM byte offset):
 */
#define CONFIG_DATA_OFFSET (SUPERCLASS_PTR_SIZE + OFFSET_FIELD_SIZE)
/*
 * The memory layout version is stored on the EEPROM -- but not in the class!
 */
#define VERSION_SIZE sizeof(uint16_t)


AbstractConfiguration::AbstractConfiguration(const uint16_t eepromOffset) {
  this->eepromOffset = eepromOffset;
}

LayoutVersion AbstractConfiguration::version() {
  LayoutVersion v;
  EEPROM.get(eepromOffset, v);
  return v;
}

void AbstractConfiguration::clear() {
  #ifdef DEBUG_CONFIG
    Serial.println(F("DEBUG_CONFIG *Clear"));
  #endif
  const uint16_t maxIndex = eepromOffset + VERSION_SIZE + (size() - CONFIG_DATA_OFFSET);
  for (uint16_t i = eepromOffset;  i < maxIndex ; i++) {
    #ifdef DEBUG_CONFIG
      Serial.print(F("DEBUG_CONFIG clr  ["));
      Serial.print(i);
      Serial.println(']');
    #endif
    EEPROM.write(i, 0x0);
  }
  // copy EEPROM 0x0 values to memory:
  readParams();
}


void AbstractConfiguration::load() {
  #ifdef DEBUG_CONFIG
   Serial.println(F("DEBUG_CONFIG *Load"));
  #endif
  if (version() != EEPROM_LAYOUT_VERSION) {
    #ifdef DEBUG_CONFIG
      Serial.println(F("DEBUG_CONFIG Updating EEPROM: Layout version"));
    #endif
    EEPROM.put(eepromOffset, EEPROM_LAYOUT_VERSION);
  }

  readParams();
  
  boolean updated;
  initParams(updated);
  if (updated) {
    #ifdef DEBUG_CONFIG
      Serial.println(F("DEBUG_CONFIG Updating EEPROM: Config values"));
    #endif
    save();
  }
}

void AbstractConfiguration::save() {
  #ifdef DEBUG_CONFIG
    Serial.println(F("DEBUG_CONFIG *Save"));
  #endif
  EEPtr e = eepromOffset + VERSION_SIZE;
  uint8_t *ptr = (uint8_t*) (this);
  ptr+= CONFIG_DATA_OFFSET;
  const uint16_t len = size() - CONFIG_DATA_OFFSET;
  for(uint16_t count = 0; count < len ; count++, e++ ) {
    #ifdef DEBUG_CONFIG
      Serial.print(F("DEBUG_CONFIG upd  ["));
      Serial.print(e);
      Serial.print(F("]="));
      Serial.println(*ptr);
    #endif
    (*e).update( *ptr++ );
  }
}

void AbstractConfiguration::print() {
  Serial.println(version());
}

void AbstractConfiguration::readParams() {
  EEPtr e = eepromOffset + VERSION_SIZE;
  uint8_t *ptr = (uint8_t*) (this);
  ptr+= CONFIG_DATA_OFFSET;
  const uint16_t len = size() - CONFIG_DATA_OFFSET;
  for(uint16_t count = 0; count < len ; count++, e++ ) {
    #ifndef DEBUG_CONFIG
      *ptr++ = *e;
    #endif
    #ifdef DEBUG_CONFIG
      *ptr = *e;
      Serial.print(F("DEBUG_CONFIG read ["));
      Serial.print(e);
      Serial.print(F("]="));
      Serial.println(*ptr++);
    #endif
  }
}
