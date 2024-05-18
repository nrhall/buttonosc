#ifndef _Network_H
#define _Network_H

#include <Ethernet.h>
#ifdef UNOWIFIR4
#include <WiFiS3.h>
#endif
#include "Config.h"

IPAddress* ip_str_to_address(const char* ip_str);
byte* mac_str_to_array(const char* mac_str);
void network_setup(Config* config);
void network_loop();

#endif