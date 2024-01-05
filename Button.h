#ifndef _Button_H
#define _Button_H

#include <OneButton.h>
#include <ezLED.h>

#define LED_HOLDTIME 125

extern "C" {
typedef void (*callback_function)(void *);
}

class Button {
private:
  const int _id;
  OneButton _button;
  ezLED _led;
  char* _context;
  callback_function _callback;

public:
  Button(const int id, const int button_pin, const int led_pin, void* context, callback_function callback);
  int id();
  void led_on(unsigned long delay = 0);
  void led_off(unsigned long delay = 0);
  void *context();
  callback_function callback();
  void on_click();
  void loop();
};

#endif
