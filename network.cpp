#include <ArduinoLog.h>
#include "network.h"

// convert an IP address into a list of ints for the Ethernet library
IPAddress* ip_str_to_address(const char* ip_str) {
  // create space for the IP array
  int* ip = malloc(4 * sizeof(int));
  sscanf(ip_str, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
  return(new IPAddress(ip[0], ip[1], ip[2], ip[3]));
}

byte* mac_str_to_array(const char* mac_str) {
  // create space for the IP array
  byte* mac = malloc(6 * sizeof(byte));
  sscanf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
  return(mac);
}

void network_setup(const char *mac_str) {
  // initialise the ethernet shield
  Ethernet.init(10);

  // start the Ethernet connection
  byte* mac_bytes = mac_str_to_array(mac_str);
  if (Ethernet.begin(mac_bytes) == 0) {
    Log.errorln(F("failed to configure Ethernet using DHCP"));
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Log.errorln(F("ethernet shield was not found."));
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Log.errorln(F("ethernet cable is not connected."));
    }
    while(1);
  }

  // output the local IP
  IPAddress localIP = Ethernet.localIP();
  Log.verbose(F("initializing Ethernet with DHCP: "));
  Log.verboseln(localIP);
}

void network_loop() {
  IPAddress localIP;
  switch (Ethernet.maintain()) {
    case 1:
      Log.errorln(F("DHCP lease renewal failed"));
      break;
    case 2:
      Log.verbose(F("DHCP lease renewed successfully: "));
      localIP = Ethernet.localIP();
      Log.verboseln(localIP);
      break;
    case 3:
      Log.errorln(F("DHCP lease rebind failed"));
      break;
    case 4:
      localIP = Ethernet.localIP();
      Log.notice(F("DHCP lease rebound successfully: "));
      Log.noticeln(localIP);
      break;
    default:
      break;
  }
}
