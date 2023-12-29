#ifndef _Config_H
#define _Config_H

class ConfigButton {
  public:
    unsigned int id;
    unsigned int led_pin;
    unsigned int button_pin;
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
    char* mac;
    unsigned int heartbeat_pin;

    String to_string();
};

class Config {
  public:
    ConfigMisc *misc;
    ConfigButton** buttons;
    ConfigTarget** targets;
    int button_count;
    int target_count;

    Config::Config(const char *filename);
    String to_string();
  };

#endif