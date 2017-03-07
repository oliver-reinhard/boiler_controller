#ifndef BC_CONFIGURATION_H_INCLUDED
  #define BC_CONFIGURATION_H_INCLUDED
  
  #include <Arduino.h>
  
  /*
   * The EEPROM storage structure for configuration parameters is as follows:
   * 
   * - Magic number (1 byte) -- enables detectiton whether the config area has been written before
   * - Version (1 byte) -- enables detection of structural changes of the config are  
   * - Parameter values (n bytes)  
   * 
   * This class implements version number storage and provides a base for extension for
   * your own parameters. When this configuration structure is first saved, the required
   * space on the EEPROM is initialized with 0x0. Parameter values are then read from the
   * 0x0-initialized EEPROM (thus resulting in zero-values) from which they will be
   * initialized with your own default values (you need to implement initParams() for that). 
   * Then these new values are stored.
   */
  class AbstractConfigParams {

    public:
      /*
       * @param eepromOffset number of bytes this object's storage is offset from the first byte of the EEPROM store.
       * @param layoutVersion static version identifier of the config data structure. Should be increased whenever the number or parameter,  their types or sizes change.
       */
      AbstractConfigParams(const uint16_t eepromOffset, const uint8_t layoutVersion);
      
      /*
       * Returns the version identifier.
       */
      uint8_t version();
      
      /*
       * Clears magic number, version and all config-parameter values stored on the EEPROM by writing 0x0 to each memory cell.
       */
      void clear();

      /*
       * Loads  parameter values from the EEPROM and applies default values to uninitialised parameters.
       */
      void load();
      
      /*
       * Saves (changed) parameter values to the EEPROM.
       */
      void save();

      /*
       * Print config parameter values to Serial.
       * Note: Consumers should override and invoke "super" first but implementation is optional.
       */
      void print();
      
      /*
       * Dynamic version of sizeof() (object size in memory); works for subclasses, too.
       */
      virtual uint16_t memSize() = 0;
      
      /*
       * Size on EEPROM.
       */
      uint16_t eepromSize();

   protected:
      /*
       * Byte offset of config space within EEPROM.
       */
      uint16_t eepromOffset;

      /*
       * Version identifier of the config data structure.
       */
      uint8_t layoutVersion;

      /*
       * Returns the "magic number" used to identify whether the config area in the EEPROM has been initialised.
       */
      uint8_t magicNumber();
      
      /*
       * Reads the configuration values (and only these) from the EEPROM. No initialisation of values is performed.
       */
      void readParams();
      
      /*
       * Ensures every parameter has been set to a value different from 0 and, if not, sets it to its default value.
       * The out parameter 'updated' tells, whether any values were affected.
       */
      virtual void initParams(boolean &updated) = 0;
  };


#endif // BC_CONFIGURATION_H_INCLUDED
