#ifndef BOILER_MESSAGE_H_INCLUDED
  #define BOILER_MESSAGE_H_INCLUDED
  
  typedef enum {
    MSG_SYSTEM_INIT = 0,      // The board is initialising from reset or power on [no parameters]
    MSG_LOG_INIT = 1,         // Log initialised [no parameters]
    MSG_ILLEGAL_TRANS = 2,    // State [state]: illegal transition attemt (event [event])
    MSG_LOG_SIZE_CHG = 3,     // Number of log entries has changed from [old] to [new])
    MSG_WATER_TEMP_SENSOR_ID_UNDEF = 4,  // Sensor ID of water-temperature sensor has not been configured. Configure and restart.
    MSG_AMBIENT_TEMP_SENSOR_ID_UNDEF = 5  // Sensor ID of ambient-temperature sensor has not been configured. Configure and restart.
  } MessageEnum;
  
#endif
