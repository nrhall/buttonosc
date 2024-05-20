#include <ArduinoLog.h>
#include "Button.h"

// wrapper to get around callback modelling in OneButton
static void callback_wrapper(void* obj) {
  ((Button*)obj)->on_click();
}

// Wired Button
WiredButton::WiredButton(const int id, const int button_pin, const int led_pin, void* context, callback_function callback) : Button(id, led_pin, context, callback), _button(OneButton(button_pin, true))
{
  _button.setClickMs(0);
  _button.setPressMs(0);
  _button.setIdleMs(0);
  _button.setDebounceMs(10);
  _button.attachLongPressStart([](void *ctx){callback_wrapper(ctx);}, this);
}

void WiredButton::hw_loop() {
  _button.tick();
}

// Wireless Button
WirelessButton::WirelessButton(const int id, const unsigned int button_intr, const unsigned long button_code, const int led_pin, void* context, callback_function callback) : Button(id, led_pin, context, callback)
{
  _last_click_time = 0;
  _button_code = button_code;
  _switch.enableReceive(button_intr);
}

void WirelessButton::hw_loop() {
  unsigned long reading = _switch.getReceivedValue();
  unsigned long sample_time = millis();

  if (reading > 0) {
    if (reading == _button_code) {
      // if the button was pressed
      //Log.traceln(F("%d %d %d %d"), reading, sample_time, _last_click_time, sample_time - _last_click_time);
      if ((sample_time - _last_click_time) > 150) {
        on_click();
      }

      _last_click_time = sample_time;
    }
  }
}

void WirelessButton::hw_reset() {
  _switch.resetAvailable();
}

// Button class
Button::Button(const int id, const int led_pin, void* context, callback_function callback) : _id(id), _led(ezLED(led_pin)), _context(context), _callback(callback)
{
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

callback_function Button::callback() {
  return _callback;
}

// callback function for button
void Button::on_click() {
  led_off();
  (*(_callback))(_context);
  led_on(LED_HOLDTIME);
}

void Button::loop() {
  // call the button loop
  hw_loop();
  // call the LED loop
  _led.loop();
}

void Button::reset() {
  // call the hardware reset
  hw_reset();
}