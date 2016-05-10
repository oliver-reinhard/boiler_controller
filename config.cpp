#include "config.h"


#define ASCII_0 48  // char(48)

String formatTemperature(Temperature t) {
  char s[9];
  byte deg = t / 100;
  byte frac = t % 100;
  s[8] = '\0';
  s[7] = 'C';
  s[6] = '\'';
  s[5] = ASCII_0 + frac % 10;
  s[4] = ASCII_0 + frac / 10;
  s[3] = '.';
  s[2] = ASCII_0 + deg % 10;
  s[1] = deg >= 10 ? ASCII_0 + deg / 10 : ' ';
  s[0] = t > 0 ? ' ' : '-';
  return s;
}


String formatTempSensorID(TempSensorID id) {
  char s[3*TEMP_SENSOR_ID_BYTES];
  byte pos = 0;
  for (byte i=0; i<TEMP_SENSOR_ID_BYTES; i++) {
    sprintf(&s[pos], "%02X", id[i]);
    pos += 2;
    if (i < TEMP_SENSOR_ID_BYTES - 1) {
      s[pos++] = '-';
    }
  }
  s[pos++] = '\0';
  return s;
}

