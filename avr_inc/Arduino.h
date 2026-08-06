#ifndef ARDUINO_H
#define ARDUINO_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define HIGH 0x1
#define LOW  0x0
#define INPUT_PULLUP 0x2
#define OUTPUT 0x1

#define F(string_literal) string_literal

inline unsigned long millis() { return 0; }
inline unsigned long micros() { return 0; }
inline void delay(unsigned long ms) { _delay_ms(ms); }
inline void delayMicroseconds(unsigned long us) { _delay_us(us); }
inline void pinMode(uint8_t pin, uint8_t mode) {}
inline void digitalWrite(uint8_t pin, uint8_t val) {}
inline int digitalRead(uint8_t pin) { return HIGH; }

class MockSerial {
public:
  void begin(unsigned long baud) {}
  void print(const char* s) {}
  void print(char c) {}
  void print(int val) {}
  void print(long val) {}
  void print(float val, int dec = 2) {}
  void println(const char* s = "") {}
  void println(int val) {}
  void println(long val) {}
  void println(float val, int dec = 2) {}
  int available() { return 0; }
  char read() { return 0; }
};

static MockSerial Serial;

#endif // ARDUINO_H
