#include <Arduino.h>
#include <ArduinoLog.h>
#include "Config.h"
#include "network.h"
#include "ButtonOSC.h"

ButtonOSC *buttonOSC;

// setup
void setup() {
  // open the serial port for debugging
  Serial.begin(1000000);
  while(!Serial){}
  delay(1000);

  // initialise logging
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  Log.setShowLevel(false);

  // load configuration
  //Config *config = new Config("config.txt", true);
  const char *json = R"(
{
  "misc": {
    "heartbeat_pin": 9
  },
  "network": {
    "ethernet": {
      "mac": "A8:61:0A:AF:00:16"
    },
    "wifi": {
    }
  },
  "buttons": [
    {
      "id": 0,
      "led_pin": 2,
      "button_pin": 54,
      "button_type": "wired",
      "osc_string": "/cue/keyboard/playhead/previous",
      "target": 0
    },
    {
      "id": 1,
      "led_pin": 3,
      "button_pin": 55,
      "button_type": "wired",
      "osc_string": "/cue/keyboard/playhead/next",
      "target": 0
    },
    {
      "id": 2,
      "led_pin": 5,
      "button_pin": 56,
      "button_type": "wired",
      "osc_string": "/cue/keyboard/go",
      "target": 0
    },
    {
      "id": 3,
      "led_pin": 6,
      "button_pin": 57,
      "button_type": "wired",
      "osc_string": "/cue/keyboard/panic",
      "target": 0
    }
  ],
  "targets": [
    {
      "id": 0,
      "server": "192.168.11.50",
      "port": 53000
    }
  ]
}
)";
const char *json1 = R"(
{
  "misc": {
    "heartbeat_pin": 2
  },
  "network": {
    "ethernet": {
      "mac": "A8:61:0A:AF:16:94"
    },
    "wifi": {
      "ssid": "HFIoT",
      "key": "Dz5gQpBx8By2Jcwbk2YE"
    }
  },
  "buttons": [
    {
      "id": 0,
      "led_pin": 1,
      "button_intr": 0,
      "button_type": "wireless",
      "button_code": 1084081,
      "osc_string": "/cue/W1/start",
      "target": 0
    },
    {
      "id": 1,
      "led_pin": 1,
      "button_intr": 0,
      "button_type": "wireless",
      "button_code": 16647425,
      "osc_string": "/cue/W1/start",
      "target": 0
    }
  ],
  "targets": [
    {
      "id": 0,
      "server": "192.168.11.50",
      "port": 53000
    }
  ]
}
)";

  Config *config = new Config(json, false);
  config->parse_json();

  // setup networking
  NetworkType network_type = network_setup(config);
  if (network_type == NONE) {
    Log.errorln(F("NET: No network found (please check the network link or WIFI configuration)"));
  }

  // setup the buttons
  buttonOSC = new ButtonOSC(config, network_type);
}

void loop() { 
  // buttonOSC loop
  buttonOSC->loop();

  // handle network loop
  network_loop();
}