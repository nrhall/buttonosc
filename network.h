#ifndef _Network_H
#define _Network_H

#include <Ethernet.h>
#ifdef ARDUINO_UNOR4_WIFI
#include <WiFiS3.h>
#endif
#include "Config.h"

typedef enum {
  NONE,
  WIRED,
  WIRELESS
} NetworkType;

IPAddress* ip_str_to_address(const char* ip_str);
byte* mac_str_to_array(const char* mac_str);
NetworkType network_setup(Config* config);
void network_loop();

#endif