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
  Config *config = new Config("config.txt");

  // setup networking
  network_setup(config);
  
  // setup the buttons
  buttonOSC = new ButtonOSC(config);
}

void loop() { 
  // buttonOSC loop
  buttonOSC->loop();

  // handle network loop
  network_loop();
}