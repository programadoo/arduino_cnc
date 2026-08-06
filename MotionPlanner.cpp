#include "MotionPlanner.h"

MotionPlanner::MotionPlanner(StepperMotor& mx, StepperMotor& my)
  : motorX(mx), motorY(my), posXObjetivo(0), posYObjetivo(0),
    dxBres(0), dyBres(0), dirXBres(0), dirYBres(0), errBres(0),
    segmentoIniciado(false), ultimoPasoUs(0) {}

void MotionPlanner::iniciarSegmento(long objX, long objY) {
  posXObjetivo = objX;
  posYObjetivo = objY;
  
  long posActualX = motorX.getPosicion();
  long posActualY = motorY.getPosicion();

  dxBres = abs(posXObjetivo - posActualX);
  dyBres = abs(posYObjetivo - posActualY);

  dirXBres = (posXObjetivo > posActualX) ? 1 : -1;
  dirYBres = (posYObjetivo > posActualY) ? 1 : -1;

  if (dxBres >= dyBres) {
    errBres = dxBres / 2;
  } else {
    errBres = dyBres / 2;
  }

  segmentoIniciado = true;
  ultimoPasoUs = micros();
}

bool MotionPlanner::update(unsigned long delayUs) {
  long posActualX = motorX.getPosicion();
  long posActualY = motorY.getPosicion();

  if (posActualX == posXObjetivo && posActualY == posYObjetivo) {
    segmentoIniciado = false;
    return true; // Objetivo alcanzado
  }

  if (!segmentoIniciado) {
    iniciarSegmento(posXObjetivo, posYObjetivo);
  }

  unsigned long ahoraUs = micros();

  if (ahoraUs - ultimoPasoUs >= delayUs) {
    ultimoPasoUs = ahoraUs;

    if (dxBres >= dyBres) {
      if (posActualX != posXObjetivo) {
        motorX.darPaso(dirXBres > 0);
        errBres -= dyBres;
        if (errBres < 0) {
          if (posActualY != posYObjetivo) {
            motorY.darPaso(dirYBres > 0);
          }
          errBres += dxBres;
        }
      }
    } else {
      if (posActualY != posYObjetivo) {
        motorY.darPaso(dirYBres > 0);
        errBres -= dxBres;
        if (errBres < 0) {
          if (posActualX != posXObjetivo) {
            motorX.darPaso(dirXBres > 0);
          }
          errBres += dyBres;
        }
      }
    }
  }

  posActualX = motorX.getPosicion();
  posActualY = motorY.getPosicion();

  if (posActualX == posXObjetivo && posActualY == posYObjetivo) {
    segmentoIniciado = false;
    return true;
  }

  return false;
}

bool MotionPlanner::isSegmentoIniciado() const {
  return segmentoIniciado;
}

void MotionPlanner::resetSegmento() {
  segmentoIniciado = false;
}
