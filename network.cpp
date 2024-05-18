#include <ArduinoLog.h>
#include "network.h"

// convert an IP address into a list of ints for the Ethernet library
IPAddress *ip_str_to_address(const char* ip_str) {
  // create space for the IP array
  int *ip = (int*)malloc(4 * sizeof(int));
  sscanf(ip_str, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
  return(new IPAddress(ip[0], ip[1], ip[2], ip[3]));
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
  Log.verboseln(F("  ip:   "));
  Log.verboseln(ip);
  Log.verboseln(F("  mask: "));
  Log.verboseln(mask);
  Log.verboseln(F("  gw:   "));
  Log.verboseln(gw);
  Log.verboseln(F("  dns:  "));
  Log.verboseln(dns);
}

void network_setup(Config *config) {
  IPAddress *ip, *mask, *gw, *dns;

  Log.traceln(F("network configuration (start)"));

  // initialise the ethernet shield
  Ethernet.init(10);
  if (Ethernet.linkStatus() == LinkON && Ethernet.hardwareStatus() != EthernetNoHardware) {
    Log.traceln(F("found ethernet hardware"));

    // wait for link
    while(Ethernet.linkStatus() == LinkOFF) {
      Log.errorln(F("ethernet cable is not connected."));
      delay(1000);
    }

    // start the Ethernet connection
    byte *mac = mac_str_to_array(config->network->ethernet->mac);

    // check for IP configuration and use it if given
    if (strlen(config->network->ethernet->ip) > 0) {
      ip = ip_str_to_address(config->network->ethernet->ip);
      mask = ip_str_to_address(config->network->ethernet->mask);
      gw = ip_str_to_address(config->network->ethernet->gw);
      dns = ip_str_to_address(config->network->ethernet->dns);

      Log.verboseln(F("initializing Ethernet with configuration: "));
      log_network_config(config->network->ethernet->ip, config->network->ethernet->mask,
                         config->network->ethernet->gw, config->network->ethernet->dns);

      Ethernet.begin(mac, *ip, *dns, *gw, *mask);
    } else {
      Log.verboseln(F("initializing Ethernet with DHCP"));
      while(Ethernet.begin(mac) == 0) {
        Log.errorln(F("failed to configure Ethernet using DHCP"));
        delay(1000);
      }

      // output the local IP
      IPAddress localIP = Ethernet.localIP();
      Log.verbose(F("IP address: "));
      Log.verboseln(localIP);
    }
  }
#ifdef UNOWIFIR4
  else if (WiFi.status() != WL_NO_SHIELD) {
    if (strlen(config->network->wifi->ssid) > 0) {
      // check for IP configuration and use it if given
      if (strlen(config->network->wifi->ip) > 0) {
        ip = ip_str_to_address(config->network->wifi->ip);
        mask = ip_str_to_address(config->network->wifi->mask);
        gw = ip_str_to_address(config->network->wifi->gw);
        dns = ip_str_to_address(config->network->wifi->dns);

        Log.verboseln(F("initializing WiFi with configuration: "));
        log_network_config(config->network->wifi->ip, config->network->wifi->mask,
                           config->network->wifi->gw, config->network->wifi->dns);

        WiFi.config(*ip, *dns, *gw, *mask);
      } else {
        Log.verboseln(F("initializing WiFi with DHCP"));
      }

      // attempt to connect to WiFi network:
      int status;
      while (status != WL_CONNECTED) {
        Serial.print("Attempting to connect to WPA SSID: ");
        Serial.println(config->network->wifi->ssid);

        // Connect to WPA/WPA2 network:
        status = WiFi.begin(config->network->wifi->ssid, config->network->wifi->key);

        // wait 10 seconds for connection:
        delay(10000);
      }
    }
  }
#endif
  else {
    Log.errorln(F("No suitable network device was found."));
  }

  Log.traceln(F("network configuration (end)"));
}

void network_loop() {
  IPAddress localIP;
  if (Ethernet.hardwareStatus() != EthernetNoHardware && Ethernet.linkStatus() != LinkOFF) {
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
}
