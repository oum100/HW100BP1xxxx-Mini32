#pragma once

#include <cstdarg>
#include <cstdint>

using byte = uint8_t;

extern unsigned long fake_millis;
extern int fake_pin_state[256];

inline unsigned long millis() {
  return fake_millis;
}

inline void digitalWrite(uint8_t pin, uint8_t value) {
  fake_pin_state[pin] = value;
}

struct SerialMock {
  template <typename... Args>
  void printf(const char *, Args...) {}

  template <typename... Args>
  void print(const Args &...) {}

  template <typename... Args>
  void println(const Args &...) {}
};

extern SerialMock Serial;
