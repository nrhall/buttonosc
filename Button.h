#ifndef _Button_H
#define _Button_H

#include <OneButton.h>
#include <RCSwitch.h>
#include <ezLED.h>

#define LED_HOLDTIME 125

extern "C" {
typedef void (*callback_function)(void *);
}

// button types
typedef enum button_type {
  BUTTON_WIRED,
  BUTTON_WIRELESS
} ButtonType;

// buttons
class Button
{
private:
  const int _id;
  ezLED _led;
  void* _context;
  callback_function _callback;

public:
  Button(const int id, const int led_pin, void* context, callback_function callback);
  int id();
  void led_on(unsigned long delay = 0);
  void led_off(unsigned long delay = 0);
  void *context();
  callback_function callback();
  void on_click();
  void loop_with_led();
  virtual void loop() = 0;
};

class WiredButton : public Button
{
private:
  OneButton _button;
public:
  WiredButton(const int id, const int button_pin, const int led_pin, void* context, callback_function callback);
  void loop();
};

class WirelessButton : public Button
{
private:
  RCSwitch _switch;
  unsigned long last_click_time;
  unsigned long _button_code;

public:
  WirelessButton(const int id, const unsigned int button_intr, const unsigned long button_code, const int led_pin, void* context, callback_function callback);
  void loop();
};

#endif
