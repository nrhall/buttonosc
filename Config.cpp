#include <ArduinoLog.h>
#include <ArduinoJson.h>
#include <SD.h>
#include "config.h"

String ConfigButton::to_string() {
  return String("Button(")
      + String("id=") + String(id)
      + String(" button=") + String(button_pin)
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
      + String("mac=") + String(mac)
      + String(" heartbeat_pin=") + String(heartbeat_pin)
      + String(")");
}

String Config::to_string() {
  String _misc = misc->to_string() + String("\n");
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
  return String("Config(\n") + _misc + _buttons + _targets + String(")");
}

// Loads the configuration from a file
Config::Config(const char *filename) {
  Sd2Card card;
  SdVolume volume;
  SdFile root;
  SdFile config_file;
  
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
  config_file.open(root, filename);
  
  // read into the json doc
  DynamicJsonDocument config_doc(2048);
  DeserializationError error = deserializeJson(config_doc, config_file);
  if (error) {
    Log.errorln(F("failed to read file: %s"), filename);
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

  // allocate space for the MAC string and copy
  int length = strlen(json_misc["mac"].as<const char *>()) + 1;
  misc->mac = (char *)malloc(length * sizeof(char));
  if (misc->mac == nullptr) {
    Log.errorln(F("unable to allocate memory for osc string"));
    while(1);
  }
  strlcpy(misc->mac, json_misc["mac"].as<const char *>(), length);
  
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
    buttons[_button]->target = obj["target"];
    
    // allocate space for the OSC string and copy
    int length = strlen(obj["osc_string"].as<const char *>()) + 1;
    buttons[_button]->osc_string = (char *)malloc(length * sizeof(char));
    if (buttons[_button]->osc_string == nullptr) {
      Log.errorln(F("unable to allocate memory for osc string"));
      while(1);
    }
    strlcpy(buttons[_button]->osc_string, obj["osc_string"].as<const char *>(), length);

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
    strlcpy(targets[_target]->server, obj["server"].as<const char *>(), length);

    _target++;
  }

  // close the file
  config_file.close();

  // tracing
  Log.traceln(to_string().c_str());
  Log.traceln(F("loading configuration (end)"));
}

