#include <ArduinoLog.h>
#include <EthernetUdp.h>
#include <OSCMessage.h>
#include "ButtonOSC.h"
#include "network.h"

EthernetUDP udp;

static void onButtonClick(void *context) {
  OSCContext* osc_context = (OSCContext*)context;

  Log.traceln(F("onButtonClick(start): %s %s %s"), osc_context->server, String(osc_context->port).c_str(), osc_context->string);

  // create the OSC message
  OSCMessage msg(osc_context->string);

  // get the target IP
  IPAddress* target_ip = ip_str_to_address(osc_context->server);

  // send the OSC message
  udp.beginPacket(*target_ip, osc_context->port);
  msg.send(udp);
  udp.endPacket();
  msg.empty();
  Log.traceln(F("onButtonClick(end): %s %s %s"), osc_context->server, String(osc_context->port).c_str(), osc_context->string);
}

ButtonOSC::ButtonOSC(Config* config) : _config(config) {
  // setup buttons
  _buttons = (Button**)malloc(sizeof(Button*) * _config->button_count);
  for (int i = 0; i < _config->button_count; i++) {
    Log.traceln(F("creating button %d/%d"), i, _config->button_count);

    // get the configuration
    ConfigButton* button = config->buttons[i];
    ConfigTarget* target = config->targets[button->target];
    
    // setup the OSC context
    OSCContext* osc_context = new OSCContext();
    osc_context->server = target->server;
    osc_context->port = target->port;
    osc_context->string = button->osc_string;
    
    // create the button/led pair with associated callback
    _buttons[i] = new Button(i, button->button_pin, button->led_pin, (void *)osc_context, onButtonClick);
  }

  // setup heartbeat
  _heartbeat_led = new ezLED(config->misc->heartbeat_pin);

  // create socket for OSC
  udp.begin(54000);
}

void ButtonOSC::loop() {
  // handle button/LED loops 
  for (int i = 0; i < _config->button_count; i++) {
    _buttons[i]->loop();
  }

  // pulse the hb LED
  static bool is_faded_in = false;
  if (_heartbeat_led->getState() == LED_IDLE) {
    if (is_faded_in == false) {
      _heartbeat_led->fade(50, 150, 2000);
      is_faded_in = true;
    } else {
      _heartbeat_led->fade(150, 50, 2000);
      is_faded_in = false;
    }
  }
  _heartbeat_led->loop();
}
