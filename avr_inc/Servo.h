#ifndef SERVO_H
#define SERVO_H

#include "Arduino.h"

class Servo {
private:
  int pin;
  int angle;
public:
  Servo() : pin(-1), angle(-1) {}
  void attach(int p) { pin = p; }
  void detach() { pin = -1; }
  void write(int a) { angle = a; }
  int read() { return angle; }
};

#endif // SERVO_H
