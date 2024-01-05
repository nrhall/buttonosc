#include <ArduinoLog.h>
#include "Button.h"

// wrapper to get around callback modelling in OneButton
static void callback_wrapper(Button* obj) {
  obj->on_click();
}

// Button class
Button::Button(const int id, const int button_pin, const int led_pin, void* context, callback_function callback) : _id(id), _button(OneButton(button_pin, true)), _led(ezLED(led_pin)), _context(context), _callback(callback) {
  // setup the button
  _button.setClickMs(0);
  _button.setPressMs(0);
  _button.setDebounceMs(50);
  _button.attachLongPressStart([](void *ctx){callback_wrapper(ctx);}, this);

  // initialise the LED state to on
  _led.turnON();
}

int Button::id() {
  return _id;
}

void Button::led_on(unsigned long delay) {
  _led.turnON(delay);
}

void Button::led_off(unsigned long delay) {
  _led.turnOFF(delay);
}

void* Button::context() {
  return _context;
}

callback_function Button::callback() {
  return _callback;
}

// callback function for OneButton
void Button::on_click() {
  Log.traceln(F("button %d pressed"), _id);
  led_off();
  (*(_callback))(_context);
  led_on(LED_HOLDTIME);
}

void Button::loop() {
  _button.tick();
  _led.loop();
}