#include <ezLED.h>
#include <OSCMessage.h>
#include "Button.h"
#include "Config.h"
#include "network.h"

class ButtonOSC {
  private:
    Button **_buttons;
    ezLED *_heartbeat_led;
    Config *_config;

  public:
    ButtonOSC(Config *config, NetworkType network_type);
    void loop();
};

struct OSCContext {
  char *server;
  unsigned int port;
  char *string;
  IPAddress *server_ip;
  NetworkType network_type;
};