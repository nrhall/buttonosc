#include <ArduinoLog.h>
#include <EthernetUdp.h>
#ifdef UNOWIFIR4
#include <WiFiUdp.h>
#endif
#include <OSCMessage.h>
#include "ButtonOSC.h"
#include "network.h"

EthernetUDP udp;

static void onButtonClick(void *context) {
  OSCContext* osc_context = (OSCContext*)context;

  Log.traceln(F("OSC: %s %s %s [%ul]"), osc_context->server, String(osc_context->port).c_str(), osc_context->string, millis());

  // create the OSC message
  OSCMessage msg(osc_context->string);

  // send the OSC message
  udp.beginPacket(*(osc_context->server_ip), osc_context->port);
  msg.send(udp);
  udp.endPacket();
  msg.empty();
  Log.traceln(F("OSC: sent [%ul]"), millis());
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
    osc_context->server_ip = ip_str_to_address(target->server);
    osc_context->port = target->port;
    osc_context->string = button->osc_string;
    
    // create the button/led pair with associated callback
    switch (button->button_type) {
      case BUTTON_WIRED:
        _buttons[i] = new WiredButton(i, button->button_pin, button->led_pin, (void *)osc_context, onButtonClick);
        break;
      case BUTTON_WIRELESS:
        _buttons[i] = new WirelessButton(i, button->button_intr, button->button_code, button->led_pin, (void *)osc_context, onButtonClick);
        break;
      default:
        Log.traceln(F("invalid button type: %d"), button->button_type);
    }
  }

  // setup heartbeat
  _heartbeat_led = new ezLED(config->misc->heartbeat_pin);

  // create socket for OSC
  udp.begin(54000);
}

void ButtonOSC::loop() {
  // handle button/LED loops 
  for (int i = 0; i < _config->button_count; i++) {
    _buttons[i]->loop_with_led();
  }

  // pulse the hb LED
  static bool is_faded_in = false;
  if (_heartbeat_led->getState() == LED_IDLE) {
    if (is_faded_in == false) {
      _heartbeat_led->fade(5, 30, 2000);
      is_faded_in = true;
    } else {
      _heartbeat_led->fade(30, 5, 2000);
      is_faded_in = false;
    }
  }
  _heartbeat_led->loop();
}
