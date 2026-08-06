#ifndef STEPPER_MOTOR_H
#define STEPPER_MOTOR_H

#include "Config.h"

class StepperMotor {
private:
  int pins[4];
  int pasoIdx;
  long posicionPasos;

public:
  StepperMotor(int pin1, int pin2, int pin3, int pin4);
  
  void begin();
  void darPaso(bool sentidoHorario);
  void apagar();
  
  long getPosicion() const;
  void setPosicion(long pos);
};

#endif // STEPPER_MOTOR_H
