#ifndef BOILER_MESSAGE_H_INCLUDED
  #define BOILER_MESSAGE_H_INCLUDED
  
  #include "Arduino.h"
  
  typedef byte MessageID;
  
  const MessageID MSG_SYSTEM_INIT = 0;  // The board is initialising from reset or power on [no parameters]
  const MessageID MSG_LOG_INIT = 1;  // Log initialised [no parameters]
  const MessageID MSG_ILLEGAL_TRANS = 2;  // State [state]: illegal transition attemt (event [event])
  const MessageID MSG_LOG_SIZE_CHG = 3;  // Number of log entries has changed from [old] to [new])

#endif