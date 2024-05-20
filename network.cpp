#include <ArduinoLog.h>
#include "network.h"

// convert an IP address into a list of ints for the Ethernet library
IPAddress *ip_str_to_address(const char* ip_str) {
  // create space for the IP array
  if (!ip_str) {
    return NULL;
  } else {
    int *ip = (int*)malloc(4 * sizeof(int));
    sscanf(ip_str, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
    return new IPAddress(ip[0], ip[1], ip[2], ip[3]);
  }
}

// convert various things into the array of ints required
int *ip_str_to_array(const char* ip_str) {
  // create space for the IP array
  int *ip = (int*)malloc(4 * sizeof(int));
  sscanf(ip_str, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
  return(ip);
}

uint8_t *mac_str_to_array(const char *mac_str) {
  // create space for the IP array
  uint8_t *mac = (uint8_t *)malloc(6 * sizeof(uint8_t));
  sscanf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
  return(mac);
}

void log_network_config(const char *ip, const char *mask, const char *gw, const char *dns) {
  Log.verboseln(F("  ip:   %s"), ip);
  Log.verboseln(F("  mask: %s"), mask);
  Log.verboseln(F("  gw:   %s"), gw);
  Log.verboseln(F("  dns:  %s"), dns);
}

NetworkType network_setup(Config *config) {
  IPAddress *ip, *mask, *gw, *dns;
  NetworkType network_type;

  Log.traceln(F("NET: Configuration (start)"));

  // initialise the ethernet shield
  Ethernet.init(10);

  // wait long enough for the link to come up first
  delay(2000);

  if (Ethernet.linkStatus() == LinkON && Ethernet.hardwareStatus() != EthernetNoHardware) {
    Log.traceln(F("NET(ETH): Found hardware"));
    network_type = WIRED;

    // wait for link
    while(Ethernet.linkStatus() == LinkOFF) {
      Log.errorln(F("NET(ETH): Cable not connected."));
      delay(1000);
    }

    // start the Ethernet connection
    byte *mac = mac_str_to_array(config->network->ethernet->mac);

    // check for IP configuration and use it if given
    if (config->network->ethernet->ip) {
      ip = ip_str_to_address(config->network->ethernet->ip);
      mask = ip_str_to_address(config->network->ethernet->mask);
      gw = ip_str_to_address(config->network->ethernet->gw);
      dns = ip_str_to_address(config->network->ethernet->dns);

      Log.verboseln(F("NET(ETH): Configuring (MANUAL): "));
      log_network_config(config->network->ethernet->ip, config->network->ethernet->mask,
                         config->network->ethernet->gw, config->network->ethernet->dns);

      Ethernet.begin(mac, *ip, *dns, *gw, *mask);
    } else {
      Log.verboseln(F("NET(ETH): Configuring (DHCP)"));
      while(Ethernet.begin(mac) == 0) {
        Log.errorln(F("NET(ETH): DHCP failed"));
        delay(1000);
      }
      Log.verbose(F("  ip: "));
      Log.verboseln(Ethernet.localIP());
      Log.verbose(F("  gw: "));
      Log.verboseln(Ethernet.gatewayIP());
    }
  }
#ifdef ARDUINO_UNOR4_WIFI
  else if (WiFi.status() != WL_NO_SHIELD) {
    network_type = WIRELESS;

    if (config->network->wifi->ssid) {
      // check for IP configuration and use it if given
      if (config->network->wifi->ip) {
        ip = ip_str_to_address(config->network->wifi->ip);
        mask = ip_str_to_address(config->network->wifi->mask);
        gw = ip_str_to_address(config->network->wifi->gw);
        dns = ip_str_to_address(config->network->wifi->dns);

        Log.verboseln(F("NET(WIFI): Configuring (MANUAL): "));
        log_network_config(config->network->wifi->ip, config->network->wifi->mask,
                           config->network->wifi->gw, config->network->wifi->dns);

        WiFi.config(*ip, *dns, *gw, *mask);
      } else {
        Log.verboseln(F("NET(WIFI): Configuring (DHCP)"));
      }

      // attempt to connect to WiFi network:
      int status;
      while (status != WL_CONNECTED) {
        Log.verboseln(F("NET(WIFI): Attempting to connect to WPA SSID: %s"), config->network->wifi->ssid);

        // Connect to WPA/WPA2 network:
        status = WiFi.begin(config->network->wifi->ssid, config->network->wifi->key);

        // wait 10 seconds for connection:
        delay(10000);
      }

      // output the local IP if DHCP
      if (!(config->network->wifi->ip)) {
        Log.verboseln(F("  ip: %s"), WiFi.localIP().toString().c_str());
        Log.verboseln(F("  gw: %s"), WiFi.gatewayIP().toString().c_str());
      }
    }
  }
#endif
  else {
    Log.errorln(F("NET: No suitable device was found."));
    network_type = NONE;
  }

  Log.traceln(F("NET: Configuration (end))"));

  return network_type;
}

void network_loop() {
  if (Ethernet.hardwareStatus() != EthernetNoHardware && Ethernet.linkStatus() != LinkOFF) {
    switch (Ethernet.maintain()) {
      case 1:
        Log.errorln(F("NET(ETH): DHCP lease renewal failed"));
        break;
      case 2:
        Log.verbose(F("NET(ETH): DHCP lease renewed successfully: "));
        Log.verboseln(Ethernet.localIP());
        break;
      case 3:
        Log.errorln(F("NET(ETH): DHCP lease rebind failed"));
        break;
      case 4:
        Log.notice(F("NET(ETH): DHCP lease rebound successfully: "));
        Log.noticeln(Ethernet.localIP());
        break;
      default:
        break;
    }
  }
}
