#include <ezLED.h>
#include "Button.h"
#include "Config.h"

class ButtonOSC {
  private:
    Button** _buttons;
    ezLED *_heartbeat_led;
    Config *_config;

  public:
    ButtonOSC(Config *config);
    void loop();
};

struct OSCContext {
  char* server;
  unsigned int port;
  char* string;
};