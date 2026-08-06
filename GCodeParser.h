#ifndef GCODE_PARSER_H
#define GCODE_PARSER_H

#include "Config.h"
#include "MotionPlanner.h"
#include "InputUtils.h"

typedef void (*CalibracionCallback)();

class GCodeParser {
private:
  MotionPlanner& planner;
  ServoZ& servoZ;
  
  char rxBuffer[64];
  int rxIndex;
  
  bool modoAbsoluto;
  float posXActualCm;
  float posYActualCm;

  float obtenerValor(const char* linea, char clave, float valorDefecto);
  bool tieneClave(const char* linea, char clave);
  void limpiarLinea(char* linea);

public:
  GCodeParser(MotionPlanner& mp, ServoZ& sz);

  void begin();
  void escucharSerial(float pasosPorCmX, float pasosPorCmY, CalibracionCallback fnCalibrar);
  void procesarComando(char* linea, float pasosPorCmX, float pasosPorCmY, CalibracionCallback fnCalibrar);
  
  void setPosicionActualCm(float x, float y);
  float getPosXActualCm() const;
  float getPosYActualCm() const;
  bool isModoAbsoluto() const;
};

#endif // GCODE_PARSER_H
