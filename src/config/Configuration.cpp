#include "Configuration.h"
#include <EEPROM.h>

//#define DEBUG_CONFIG
//#define DEBUG_CONFIG_DETAIL

/*
 * The magic number is the first byte of the config area. During startup it enables the detection whether
 * the config area has been initialised before. The EEPROM values of an Arduino board being read for the
 * very first time *cannot be assumed to be 0* !!
 */
const uint8_t MAGIC_NUMBER = 123;

#define MAGIC_NUMBER_SIZE sizeof(uint8_t)
#define VERSION_NUMBER_SIZE sizeof(uint8_t)
#define EEPROM_PARAM_OFFSET (MAGIC_NUMBER_SIZE + VERSION_NUMBER_SIZE)

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
#define CONFIG_DATA_OFFSET (SUPERCLASS_PTR_SIZE + OFFSET_FIELD_SIZE + VERSION_NUMBER_SIZE)


AbstractConfigParams::AbstractConfigParams(const uint16_t eepromOffset, const uint8_t layoutVersion) {
  this->eepromOffset = eepromOffset;
  this->layoutVersion = layoutVersion;
}

uint8_t AbstractConfigParams::magicNumber() {
  uint8_t v;
  EEPROM.get(eepromOffset, v);
  return v;
}

uint8_t AbstractConfigParams::version() {
  uint8_t v;
  EEPROM.get(eepromOffset + MAGIC_NUMBER_SIZE, v);
  return v;
}

void AbstractConfigParams::clear() {
  #ifdef DEBUG_CONFIG
    Serial.println(F("DEBUG_CONFIG *Clear"));
  #endif
  const uint16_t maxIndex = eepromOffset + eepromSize();
  for (uint16_t i = eepromOffset;  i < maxIndex ; i++) {
    #ifdef DEBUG_CONFIG_DETAIL
      Serial.print(F("DEBUG_CONFIG_DETAIL clr  ["));
      Serial.print(i);
      Serial.println(']');
    #endif
    EEPROM.write(i, 0x0);
  }
  // copy EEPROM 0x0 values to memory:
  readParams();
}


void AbstractConfigParams::load() {
  #ifdef DEBUG_CONFIG
   Serial.println(F("DEBUG_CONFIG *Load"));
  #endif
  
  if (magicNumber() != MAGIC_NUMBER) {
    #ifdef DEBUG_CONFIG
      Serial.println(F("DEBUG_CONFIG Clearing EEPROM (first use)"));
    #endif
    clear();
    EEPROM.put(eepromOffset, MAGIC_NUMBER);
    EEPROM.put(eepromOffset + MAGIC_NUMBER_SIZE, layoutVersion);
    
  } else if (version() != layoutVersion) {
    #ifdef DEBUG_CONFIG
      Serial.println(F("DEBUG_CONFIG Updating EEPROM: Clearing config block, updating layout version"));
    #endif
    EEPROM.put(eepromOffset + MAGIC_NUMBER_SIZE, layoutVersion);
    readParams();

  } else {
    readParams();
  }
  
  boolean updated;
  initParams(updated);
  if (updated) {
    #ifdef DEBUG_CONFIG
      Serial.println(F("DEBUG_CONFIG Updating EEPROM: Config values"));
    #endif
    save();
  }
}

void AbstractConfigParams::save() {
  #ifdef DEBUG_CONFIG
    Serial.println(F("DEBUG_CONFIG *Save"));
  #endif
  EEPtr e = eepromOffset + EEPROM_PARAM_OFFSET;
  uint8_t *ptr = (uint8_t*) (this);
  ptr+= CONFIG_DATA_OFFSET;
  const uint16_t len = memSize() - CONFIG_DATA_OFFSET;
  for(uint16_t count = 0; count < len ; count++, e++ ) {
    #ifdef DEBUG_CONFIG_DETAIL
      Serial.print(F("DEBUG_CONFIG_DETAIL upd  ["));
      Serial.print(e);
      Serial.print(F("]="));
      Serial.println(*ptr);
    #endif
    (*e).update( *ptr++ );
  }
}


uint16_t AbstractConfigParams::eepromSize() {
  return EEPROM_PARAM_OFFSET + (memSize() - CONFIG_DATA_OFFSET);
}

void AbstractConfigParams::print() {
  Serial.println(version());
}

void AbstractConfigParams::readParams() {
  EEPtr e = eepromOffset + EEPROM_PARAM_OFFSET;
  uint8_t *ptr = (uint8_t*) (this);
  ptr+= CONFIG_DATA_OFFSET;
  const uint16_t len = memSize() - CONFIG_DATA_OFFSET;
  for(uint16_t count = 0; count < len ; count++, e++ ) {
    #ifndef DEBUG_CONFIG_DETAIL
      *ptr++ = *e;
    #endif
    #ifdef DEBUG_CONFIG_DETAIL
      *ptr = *e;
      Serial.print(F("DEBUG_CONFIG_DETAIL read ["));
      Serial.print(e);
      Serial.print(F("]="));
      Serial.println(*ptr++);
    #endif
  }
}
