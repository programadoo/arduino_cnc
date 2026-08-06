#ifndef MOTION_PLANNER_H
#define MOTION_PLANNER_H

#include "Config.h"
#include "StepperMotor.h"

class MotionPlanner {
private:
  StepperMotor& motorX;
  StepperMotor& motorY;

  long posXObjetivo;
  long posYObjetivo;
  long dxBres;
  long dyBres;
  int dirXBres;
  int dirYBres;
  long errBres;
  bool segmentoIniciado;
  unsigned long ultimoPasoUs;

public:
  MotionPlanner(StepperMotor& mx, StepperMotor& my);

  void iniciarSegmento(long objX, long objY);
  bool update(unsigned long delayUs);
  bool isSegmentoIniciado() const;
  void resetSegmento();
};

#endif // MOTION_PLANNER_H
