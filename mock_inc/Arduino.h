#ifndef ARDUINO_H
#define ARDUINO_H

#include <iostream>
#include <cstdint>
#include <cmath>
#include <cstring>
#include <cstdlib>

#define HIGH 0x1
#define LOW  0x0
#define INPUT_PULLUP 0x2
#define OUTPUT 0x1

#define F(string_literal) string_literal

inline unsigned long& get_mock_millis() {
  static unsigned long m = 1000;
  return m;
}

inline unsigned long& get_mock_micros() {
  static unsigned long u = 1000000;
  return u;
}

inline unsigned long millis() { return get_mock_millis(); }
inline unsigned long micros() { return get_mock_micros(); }
inline void delay(unsigned long ms) { get_mock_millis() += ms; get_mock_micros() += ms * 1000; }
inline void delayMicroseconds(unsigned long us) { get_mock_micros() += us; }
inline void pinMode(int pin, int mode) {}
inline void digitalWrite(int pin, int val) {}
inline int digitalRead(int pin) { return HIGH; }

class MockSerial {
public:
  void begin(unsigned long baud) {}
  void print(const char* s) { std::cout << s; }
  void print(char c) { std::cout << c; }
  void print(int val) { std::cout << val; }
  void print(long val) { std::cout << val; }
  void print(float val, int dec = 2) { std::cout << val; }
  void println(const char* s = "") { std::cout << s << std::endl; }
  void println(int val) { std::cout << val << std::endl; }
  void println(long val) { std::cout << val << std::endl; }
  void println(float val, int dec = 2) { std::cout << val << std::endl; }
  int available() { return 0; }
  char read() { return 0; }
};

static MockSerial Serial;

#endif // ARDUINO_H
