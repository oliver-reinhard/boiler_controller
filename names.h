#ifndef BOILER_NAMES_H_INCLUDED
  #define BOILER_NAMES_H_INCLUDED

  #include "config.h"
  #include "state.h"
  #include "control.h"

  String getName(ConfigParamEnum literal);
  String getName(StateEnum literal);
  String getName(EventEnum literal);
  String getName(UserCommandEnum literal);
  String getName(SensorStatusEnum literal);

#endif
