#ifndef _Network_H
#define _Network_H

#include <Ethernet.h>

IPAddress* ip_str_to_address(const char* ip_str);
byte* mac_str_to_array(const char* mac_str);
void network_setup(const char* mac);
void network_loop();

#endif