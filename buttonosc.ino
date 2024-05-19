#include <Arduino.h>
#include <ArduinoLog.h>
#include "Config.h"
#include "network.h"
#include "ButtonOSC.h"

ButtonOSC *buttonOSC;

// setup
void setup() {
  // open the serial port for debugging
  Serial.begin(9600);
  while(!Serial && !Serial.available()){}

  // initialise logging
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);

  // load configuration
  //Config *config = new Config("config.txt", true);
  const char *json = R"(
{
  "misc": {
    "heartbeat_pin": 4
  },
  "network": {
    "ethernet": {
      "mac": "A8:61:0A:AF:00:16",
      "ip": "",
      "mask": "",
      "gw": "",
      "dns": ""
    },
    "wifi": {
      "ssid": "HFIoT",
      "key": "Dz5gQpBx8By2Jcwbk2YE",
      "ip": "",
      "mask": "",
      "gw": "",
      "dns": ""
    }
  },
  "buttons": [
    {
      "id": 0,
      "led_pin": 2,
      "button_intr": 0,
      "button_type": "wireless",
      "button_code": 1084081,
      "osc_string": "/cue/W1/start",
      "target": 0
    }
  ],
  "targets": [
    {
      "id": 0,
      "server": "10.0.1.227",
      "port": 53000
    }
  ]
}
)";
  Config *config = new Config(json, false);
  config->parse_json();

  // setup networking
  NetworkType network_type = network_setup(config);

  // setup the buttons
  buttonOSC = new ButtonOSC(config, network_type);
}

void loop() { 
  // buttonOSC loop
  buttonOSC->loop();

  // handle network loop
  network_loop();
}