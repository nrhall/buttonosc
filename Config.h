#ifndef _Config_H
#define _Config_H

#include <ArduinoJson.h>
#include "Button.h"

class ConfigButton {
  public:
    unsigned int id;
    unsigned int led_pin;
    unsigned int button_pin;
    unsigned int button_intr;
    unsigned long button_code;
    ButtonType button_type;
    char* osc_string;
    unsigned int target;

    String to_string();
};

class ConfigTarget {
  public:
    unsigned int id;
    char* server;
    unsigned int port;

    String to_string();
};

class ConfigMisc {
  public:
    unsigned int heartbeat_pin;

    String to_string();
};

class ConfigNetworkEthernet {
  public:
    char* mac;
    char* ip;
    char* mask;
    char* gw;
    char* dns;

    String to_string();
};

class ConfigNetworkWifi {
  public:
    char* ssid;
    char* key;
    char* ip;
    char* mask;
    char* gw;
    char* dns;

    String to_string();
};

class ConfigNetwork {
  public:
    ConfigNetworkEthernet* ethernet;
    ConfigNetworkWifi* wifi;

    String to_string();
};

class Config {
  private:
    const char *buffer;
  public:
    ConfigMisc* misc;
    ConfigNetwork* network;
    ConfigButton** buttons;
    ConfigTarget** targets;
    int button_count;
    int target_count;

    Config(const char *config, const bool read_from_sd);
    void parse_json();
    char *copy_value(JsonObject obj, const char *key);
    String to_string();
};

#endif