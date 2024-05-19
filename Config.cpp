#include <ArduinoLog.h>
#include <ArduinoJson.h>
#include <SD.h>
#include <string.h>
#include "config.h"

String ConfigButton::to_string() {
  return String("Button(")
      + String("id=") + String(id)
      + String(" button_type=") + String(button_type)
      + String(" button_pin=") + String(button_pin)
      + String(" button_intr=") + String(button_intr)
      + String(" button_code=") + String(button_code)
      + String(" led=") + String(led_pin)
      + String(" osc=") + osc_string
      + String(" target=") + String(target)
      + String(")");
}

String ConfigTarget::to_string() {
  return String("Target(")
      + String("id=") + String(id)
      + String(" server=") + String(server)
      + String(" port=") + String(port)
      + String(")");
}

String ConfigMisc::to_string() {
  return String("Misc(")
      + String("heartbeat_pin=") + String(heartbeat_pin)
      + String(")");
}

String ConfigNetwork::to_string() {
  String _ethernet = ethernet->to_string() + String("\n");
  String _wifi = wifi->to_string() + String("\n");
  return String("Network(\n") + _ethernet + _wifi + String(")");
}

String ConfigNetworkEthernet::to_string() {
  return String("Network/Ethernet(")
      + String("mac=") + String(mac)
      + String(" ip=") + String(ip)
      + String(" mask=") + String(mask)
      + String(" gw=") + String(gw)
      + String(" dns=") + String(dns)
      + String(")");
}

String ConfigNetworkWifi::to_string() {
  return String("Network/Wifi(")
      + String("ssid=") + String(ssid)
      + String(" key=") + String(key)
      + String(" ip=") + String(ip)
      + String(" mask=") + String(mask)
      + String(" gw=") + String(gw)
      + String(" dns=") + String(dns)
      + String(")");
}

String Config::to_string() {
  String _misc = misc->to_string() + String("\n");
  String _network = network->to_string() + String("\n");
  String _buttons;
  for (int i = 0; i < button_count; i++) {
    _buttons += buttons[i]->to_string();
    _buttons += String("\n");
  }
  String _targets;
  for (int i = 0; i < target_count; i++) {
    _targets += targets[i]->to_string();
    _targets += String("\n");
  }
  return String("Config(\n") + _misc + _network + _buttons + _targets + String(")");
}

// Loads the configuration from a file
const char *read_file_from_sd(const char *filename) {
  Sd2Card card;
  SdVolume volume;
  SdFile root;
  SdFile _config_file;
  SDFile config_file;
  
  // tracing
  Log.traceln(F("loading configuration (start)"));

  // initialise the SD card
  Log.traceln(F("initializing SD card"));
  if (!card.init(SPI_HALF_SPEED, 4)) {
    Log.errorln(F("initialization failed!"));
    while (1);
  }

  // get the volume and root dir from the card
  if (!volume.init(card)) {
    Log.errorln(F("could not find FAT16/FAT32 partition on SD card"));
    while (1);
  }
  root.openRoot(volume);
  
  // open the required configuration file
  _config_file.open(root, filename);
  config_file = SDFile(_config_file, filename);

  // read the file into a string
  unsigned long size = config_file.size();
  char *buffer = new char[size];
  int error = config_file.read(buffer, size);
  if (error) {
    Log.errorln(F("failed to read file: %s"), filename);
    while(1);
  }

  config_file.close();

  return buffer;
}

void Config::parse_json()
{
  int length;

  // read into the json doc
  DynamicJsonDocument config_doc(2048);
  DeserializationError error = deserializeJson(config_doc, buffer);
  if (error) {
    Log.errorln(F("failed to deserialize JSON"));
    while(1);
  }

  // parse the json doc
  JsonObject json_root = config_doc.as<JsonObject>();
  if (json_root == nullptr || 
       !json_root.containsKey("misc") ||
       !json_root.containsKey("buttons") ||
       !json_root.containsKey("targets")) {
    Log.errorln(F("JSON document does exist or does not contain required keys: 'misc', 'buttons' and/or 'targets'"));
    while(1);
  }

  // get the misc config
  JsonObject json_misc = json_root["misc"].as<JsonObject>();
  if (json_misc == nullptr) {
    Log.errorln(F("JSON document does not have a 'misc' object"));
    while(1);
  }

  misc = (ConfigMisc*)malloc(sizeof(ConfigMisc));
  if (misc == nullptr) {
    Log.errorln(F("unable to allocate memory for misc config"));
    while(1);
  }
  
  // copy the integer config
  misc->heartbeat_pin = json_misc["heartbeat_pin"];

  // get the network config
  JsonObject json_network = json_root["network"].as<JsonObject>();
  if (json_network == nullptr) {
    Log.errorln(F("JSON document does not have a 'network' object"));
    while(1);
  }

  // network config
  network = (ConfigNetwork*)malloc(sizeof(ConfigNetwork));
  if (network == nullptr) {
    Log.errorln(F("unable to allocate memory for network config"));
    while(1);
  }

  // ethernet config
  network->ethernet = (ConfigNetworkEthernet*)malloc(sizeof(ConfigNetworkEthernet));
  if (network->ethernet == nullptr) {
    Log.errorln(F("unable to allocate memory for ethernet network config"));
    while(1);
  }

  // allocate space for the MAC string and copy
  length = strlen(json_network["ethernet"]["mac"].as<const char *>()) + 1;
  network->ethernet->mac = (char *)malloc(length * sizeof(char));
  if (network->ethernet->mac == nullptr) {
    Log.errorln(F("unable to allocate memory for MAC address string"));
    while(1);
  }
  strncpy(network->ethernet->mac, json_network["ethernet"]["mac"].as<const char *>(), length);
  
  // allocate space for the IP config and copy
  length = strlen(json_network["ethernet"]["ip"].as<const char *>()) + 1;
  network->ethernet->ip = (char *)malloc(length * sizeof(char));
  if (network->ethernet->ip == nullptr) {
    Log.errorln(F("unable to allocate memory for ip string"));
    while(1);
  }
  strncpy(network->ethernet->ip, json_network["ethernet"]["ip"].as<const char *>(), length);

  length = strlen(json_network["ethernet"]["mask"].as<const char *>()) + 1;
  network->ethernet->mask = (char *)malloc(length * sizeof(char));
  if (network->ethernet->mask == nullptr) {
    Log.errorln(F("unable to allocate memory for netmask string"));
    while(1);
  }
  strncpy(network->ethernet->mask, json_network["ethernet"]["mask"].as<const char *>(), length);

  length = strlen(json_network["ethernet"]["gw"].as<const char *>()) + 1;
  network->ethernet->gw = (char *)malloc(length * sizeof(char));
  if (network->ethernet->gw == nullptr) {
    Log.errorln(F("unable to allocate memory for gateway string"));
    while(1);
  }
  strncpy(network->ethernet->gw, json_network["ethernet"]["gw"].as<const char *>(), length);

  // WIFI config
  network->wifi = (ConfigNetworkWifi*)malloc(sizeof(ConfigNetworkWifi));
  if (network->wifi == nullptr) {
    Log.errorln(F("unable to allocate memory for WIFI network config"));
    while(1);
  }

  // allocate space for the SSID string and copy
  length = strlen(json_network["wifi"]["ssid"].as<const char *>()) + 1;
  network->wifi->ssid = (char *)malloc(length * sizeof(char));
  if (network->wifi->ssid == nullptr) {
    Log.errorln(F("unable to allocate memory for SSID string"));
    while(1);
  }
  strncpy(network->wifi->ssid, json_network["wifi"]["ssid"].as<const char *>(), length);

  // allocate space for the SSID key and copy
  length = strlen(json_network["wifi"]["key"].as<const char *>()) + 1;
  network->wifi->key = (char *)malloc(length * sizeof(char));
  if (network->wifi->key == nullptr) {
    Log.errorln(F("unable to allocate memory for SSID key string"));
    while(1);
  }
  strncpy(network->wifi->key, json_network["wifi"]["key"].as<const char *>(), length);

  // allocate space for the IP config and copy
  length = strlen(json_network["wifi"]["ip"].as<const char *>()) + 1;
  network->wifi->ip = (char *)malloc(length * sizeof(char));
  if (network->wifi->ip == nullptr) {
    Log.errorln(F("unable to allocate memory for ip string"));
    while(1);
  }
  strncpy(network->wifi->ip, json_network["wifi"]["ip"].as<const char *>(), length);

  length = strlen(json_network["wifi"]["mask"].as<const char *>()) + 1;
  network->wifi->mask = (char *)malloc(length * sizeof(char));
  if (network->wifi->mask == nullptr) {
    Log.errorln(F("unable to allocate memory for netmask string"));
    while(1);
  }
  strncpy(network->wifi->mask, json_network["wifi"]["mask"].as<const char *>(), length);

  length = strlen(json_network["wifi"]["gw"].as<const char *>()) + 1;
  network->wifi->gw = (char *)malloc(length * sizeof(char));
  if (network->wifi->gw == nullptr) {
    Log.errorln(F("unable to allocate memory for gateway string"));
    while(1);
  }
  strncpy(network->wifi->gw, json_network["wifi"]["gw"].as<const char *>(), length);

  // get the buttons
  JsonArray json_buttons = json_root["buttons"].as<JsonArray>();
  if (json_buttons == nullptr) {
    Log.errorln(F("JSON document does not have a 'buttons' array"));
    while(1);
  }

  button_count = json_buttons.size();
  buttons = (ConfigButton**)malloc(button_count * sizeof(ConfigButton*));
  if (buttons == nullptr) {
    Log.errorln(F("unable to allocate memory for buttons config"));
    while(1);
  }
  
  int _button = 0;
  for (JsonObject obj : json_buttons) {
    // allocate space
    buttons[_button] = (ConfigButton*)malloc(sizeof(ConfigButton));
    if (buttons[_button] == nullptr) {
      Log.errorln(F("unable to allocate memory for button"));
      while(1);
    }

    // copy the integer values
    buttons[_button]->id = obj["id"];
    buttons[_button]->led_pin = obj["led_pin"];
    buttons[_button]->button_pin = obj["button_pin"];
    buttons[_button]->button_intr = obj["button_intr"];
    buttons[_button]->button_code = obj["button_code"];
    buttons[_button]->target = obj["target"];
    
    // allocate space for the OSC string and copy
    int length = strlen(obj["osc_string"].as<const char *>()) + 1;
    buttons[_button]->osc_string = (char *)malloc(length * sizeof(char));
    if (buttons[_button]->osc_string == nullptr) {
      Log.errorln(F("unable to allocate memory for osc string"));
      while(1);
    }
    strncpy(buttons[_button]->osc_string, obj["osc_string"].as<const char *>(), length);

    // get the button type
    if (strncmp(obj["button_type"], "wired", 5) == 0) {
      buttons[_button]->button_type = BUTTON_WIRED;
    } else if (strncmp(obj["button_type"], "wireless", 8) == 0) {
      buttons[_button]->button_type = BUTTON_WIRELESS;
    } else {
      Log.errorln(F("incorrect value for 'button_type' configuration"));
    }

    _button++;
  }

  // get the targets
  JsonArray json_targets = json_root["targets"].as<JsonArray>();
  if (json_targets == nullptr) {
    Log.errorln(F("JSON document does not have a 'targets' array"));
    while(1);
  }

  target_count = json_targets.size();
  targets = (ConfigTarget**)malloc(target_count * sizeof(ConfigTarget*));
  if (targets == nullptr) {
    Log.errorln(F("unable to allocate memory for targets config"));
    while(1);
  }

  int _target = 0;
  for (JsonObject obj : json_targets) {
    // allocate space
    targets[_target] = (ConfigTarget*)malloc(sizeof(ConfigTarget));
    if (targets[_target] == nullptr) {
      Log.errorln(F("unable to allocate memory for target"));
      while(1);
    }
    
    // copy the integer values
    targets[_target]->id = obj["id"];
    targets[_target]->port = obj["port"];

    // allocate space for the server string and copy
    int length = strlen(obj["server"].as<const char *>()) + 1;
    targets[_target]->server = (char *)malloc(length * sizeof(char));
    if (targets[_target]->server == nullptr) {
      Log.errorln(F("unable to allocate memory for server string"));
      while(1);
    }
    strncpy(targets[_target]->server, obj["server"].as<const char *>(), length);

    _target++;
  }

  // tracing
  //Log.traceln(to_string().c_str());
  Log.traceln(F("loading configuration (end)"));
}

Config::Config(const char *config, const bool read_from_sd)
{
  if (read_from_sd) {
    buffer = read_file_from_sd(config);
  } else {
    buffer = config;
  }
}
