#include "Arduino.h"

extern void setup();
extern void loop();

int main(void) {
  setup();
  for (;;) {
    loop();
  }
  return 0;
}
