#include <ArduinoLog.h>
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
      + String(" led_pin=") + String(led_pin)
      + String(" osc_string=") + String(osc_string ? osc_string : "")
      + String(" target=") + String(target)
      + String(")");
}

String ConfigTarget::to_string() {
  return String("Target(")
      + String("id=") + String(id)
      + String(" server=") + String(server ? server : "")
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
      + String("mac=") + String(mac ? mac : "")
      + String(" ip=") + String(ip ? ip : "")
      + String(" mask=") + String(mask ? mask : "")
      + String(" gw=") + String(gw ? gw : "")
      + String(" dns=") + String(dns ? dns : "")
      + String(")");
}

String ConfigNetworkWifi::to_string() {
  return String("Network/Wifi(")
      + String("ssid=") + String(ssid ? ssid : "")
      + String(" key=") + String(key ? key : "")
      + String(" ip=") + String(ip ? ip : "")
      + String(" mask=") + String(mask ? mask : "")
      + String(" gw=") + String(gw ? gw : "")
      + String(" dns=") + String(dns ? dns : "")
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
  Log.traceln(F("CONFIG: Loading configuration from SD"));

  // initialise the SD card
  Log.traceln(F("SD: Initializing SD card"));
  if (!card.init(SPI_HALF_SPEED, 4)) {
    Log.errorln(F("SD: Initialization failed!"));
    while (1);
  }

  // get the volume and root dir from the card
  if (!volume.init(card)) {
    Log.errorln(F("SD: Could not find FAT16/FAT32 partition on SD card"));
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
    Log.errorln(F("SD Failed to read file: %s"), filename);
    while(1);
  }

  config_file.close();

  return buffer;
}

char *Config::copy_value(JsonObject obj, const char *key)
{
  const char *source;
  char *dest;
  int length;

  if (obj.containsKey(key)) {
    source = obj[key];
    length = strlen(source) + 1;
    dest = (char *)malloc(length * sizeof(char));
    if (dest == nullptr) {
      Log.errorln(F("CONFIG: unable to allocate memory for string"));
      while(1);
    }
    strncpy(dest, source, length);
  } else {
    dest = NULL;
  }

  return dest;
}

void Config::parse_json()
{
  int length;

  Log.traceln(F("CONFIG: Loading JSON"));

  // read into the json doc
  DynamicJsonDocument config_doc(2048);
  DeserializationError error = deserializeJson(config_doc, buffer);
  if (error) {
    Log.errorln(F("CONFIG: failed to deserialize JSON"));
    while(1);
  }

  // parse the json doc
  JsonObject json_root = config_doc.as<JsonObject>();
  if (json_root == nullptr || 
       !json_root.containsKey("misc") ||
       !json_root.containsKey("network") ||
       !json_root.containsKey("buttons") ||
       !json_root.containsKey("targets")) {
    Log.errorln(F("CONFIG: JSON document does exist or does not contain required keys: 'misc', 'buttons' and/or 'targets'"));
    while(1);
  }

  // get the misc config
  JsonObject json_misc = json_root["misc"].as<JsonObject>();
  if (json_misc == nullptr) {
    Log.errorln(F("CONFIG: JSON document does not have a 'misc' object"));
    while(1);
  }

  misc = (ConfigMisc*)malloc(sizeof(ConfigMisc));
  if (misc == nullptr) {
    Log.errorln(F("CONFIG: Unable to allocate memory for misc config"));
    while(1);
  }
  
  // copy the integer config
  misc->heartbeat_pin = json_misc["heartbeat_pin"];

  // get the network config
  JsonObject json_network = json_root["network"].as<JsonObject>();
  if (json_network == nullptr) {
    Log.errorln(F("CONFIG: JSON document does not have a 'network' object"));
    while(1);
  }

  // network config
  network = (ConfigNetwork*)malloc(sizeof(ConfigNetwork));
  if (network == nullptr) {
    Log.errorln(F("CONFIG: Unable to allocate memory for network config"));
    while(1);
  }

  // ethernet config
  network->ethernet = (ConfigNetworkEthernet*)malloc(sizeof(ConfigNetworkEthernet));
  if (network->ethernet == nullptr) {
    Log.errorln(F("CONFIG: Unable to allocate memory for ethernet network config"));
    while(1);
  }

  network->ethernet->mac = copy_value(json_network["ethernet"], "mac");
  network->ethernet->ip = copy_value(json_network["ethernet"], "ip");
  network->ethernet->mask = copy_value(json_network["ethernet"], "mask");
  network->ethernet->gw = copy_value(json_network["ethernet"], "gw");
  network->ethernet->dns = copy_value(json_network["ethernet"], "dns");

  // WIFI config
  network->wifi = (ConfigNetworkWifi*)malloc(sizeof(ConfigNetworkWifi));
  if (network->wifi == nullptr) {
    Log.errorln(F("CONFIG: Unable to allocate memory for WIFI network config"));
    while(1);
  }

  network->wifi->ssid = copy_value(json_network["wifi"], "ssid");
  network->wifi->key = copy_value(json_network["wifi"], "key");
  network->wifi->ip = copy_value(json_network["wifi"], "ip");
  network->wifi->mask = copy_value(json_network["wifi"], "mask");
  network->wifi->gw = copy_value(json_network["wifi"], "gw");
  network->wifi->dns = copy_value(json_network["wifi"], "dns");

  // get the buttons
  JsonArray json_buttons = json_root["buttons"].as<JsonArray>();
  if (json_buttons == nullptr) {
    Log.errorln(F("CONFIG: JSON document does not have a 'buttons' array"));
    while(1);
  }

  button_count = json_buttons.size();
  buttons = (ConfigButton**)malloc(button_count * sizeof(ConfigButton*));
  if (buttons == nullptr) {
    Log.errorln(F("CONFIG: Unable to allocate memory for buttons config"));
    while(1);
  }
  
  int _button = 0;
  for (JsonObject obj : json_buttons) {
    // allocate space
    buttons[_button] = (ConfigButton*)malloc(sizeof(ConfigButton));
    if (buttons[_button] == nullptr) {
      Log.errorln(F("CONFIG: Unable to allocate memory for button"));
      while(1);
    }

    // copy the integer values
    buttons[_button]->id = obj["id"];
    buttons[_button]->led_pin = obj["led_pin"];
    buttons[_button]->button_pin = obj["button_pin"];
    buttons[_button]->button_intr = obj["button_intr"];
    buttons[_button]->button_code = obj["button_code"];
    buttons[_button]->target = obj["target"];
    buttons[_button]->osc_string = copy_value(obj, "osc_string");

    // get the button type
    if (strncmp(obj["button_type"], "wired", 5) == 0) {
      buttons[_button]->button_type = BUTTON_WIRED;
    } else if (strncmp(obj["button_type"], "wireless", 8) == 0) {
      buttons[_button]->button_type = BUTTON_WIRELESS;
    } else {
      Log.errorln(F("CONFIG: Incorrect value for 'button_type' configuration"));
    }

    _button++;
  }

  // get the targets
  JsonArray json_targets = json_root["targets"].as<JsonArray>();
  if (json_targets == nullptr) {
    Log.errorln(F("CONFIG: JSON document does not have a 'targets' array"));
    while(1);
  }

  target_count = json_targets.size();
  targets = (ConfigTarget**)malloc(target_count * sizeof(ConfigTarget*));
  if (targets == nullptr) {
    Log.errorln(F("CONFIG: Unable to allocate memory for targets config"));
    while(1);
  }

  int _target = 0;
  for (JsonObject obj : json_targets) {
    // allocate space
    targets[_target] = (ConfigTarget*)malloc(sizeof(ConfigTarget));
    if (targets[_target] == nullptr) {
      Log.errorln(F("CONFIG: Unable to allocate memory for target"));
      while(1);
    }
    
    // copy the target values
    targets[_target]->id = obj["id"];
    targets[_target]->port = obj["port"];
    targets[_target]->server = copy_value(obj, "server");

    _target++;
  }

  // tracing
  Log.traceln(to_string().c_str());
  Log.traceln(F("CONFIG: Loading configuration (end)"));
}

Config::Config(const char *config, const bool read_from_sd)
{
  if (read_from_sd) {
    buffer = read_file_from_sd(config);
  } else {
    buffer = config;
  }
}
