#include "StepperMotor.h"

StepperMotor::StepperMotor(int pin1, int pin2, int pin3, int pin4) 
  : pasoIdx(0), posicionPasos(0) {
  pins[0] = pin1;
  pins[1] = pin2;
  pins[2] = pin3;
  pins[3] = pin4;
}

void StepperMotor::begin() {
  for (int i = 0; i < 4; i++) {
    pinMode(pins[i], OUTPUT);
  }
  apagar();
}

void StepperMotor::darPaso(bool sentidoHorario) {
  pasoIdx = sentidoHorario ? (pasoIdx + 1) % 8 : (pasoIdx - 1 + 8) % 8;
  
  for (int i = 0; i < 4; i++) {
    digitalWrite(pins[i], PASO_SECUENCIA[pasoIdx][i]);
  }

  if (sentidoHorario) {
    posicionPasos++;
  } else {
    posicionPasos--;
  }
}

void StepperMotor::apagar() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(pins[i], LOW);
  }
}

long StepperMotor::getPosicion() const {
  return posicionPasos;
}

void StepperMotor::setPosicion(long pos) {
  posicionPasos = pos;
}
