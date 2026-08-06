#include "GCodeParser.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

GCodeParser::GCodeParser(MotionPlanner& mp, ServoZ& sz)
  : planner(mp), servoZ(sz), rxIndex(0), modoAbsoluto(true),
    posXActualCm(0.0f), posYActualCm(0.0f) {
  memset(rxBuffer, 0, sizeof(rxBuffer));
}

void GCodeParser::begin() {
  rxIndex = 0;
  modoAbsoluto = true;
  posXActualCm = 0.0f;
  posYActualCm = 0.0f;
  memset(rxBuffer, 0, sizeof(rxBuffer));
}

void GCodeParser::setPosicionActualCm(float x, float y) {
  posXActualCm = x;
  posYActualCm = y;
}

float GCodeParser::getPosXActualCm() const {
  return posXActualCm;
}

float GCodeParser::getPosYActualCm() const {
  return posYActualCm;
}

bool GCodeParser::isModoAbsoluto() const {
  return modoAbsoluto;
}

void GCodeParser::limpiarLinea(char* linea) {
  // Eliminar comentarios entre parentesis () o despues de ;
  char* ptrComentario1 = strchr(linea, ';');
  if (ptrComentario1) *ptrComentario1 = '\0';
  
  char* ptrComentario2 = strchr(linea, '(');
  if (ptrComentario2) *ptrComentario2 = '\0';

  // Convertir a mayusculas
  for (int i = 0; linea[i] != '\0'; i++) {
    linea[i] = toupper(linea[i]);
  }
}

bool GCodeParser::tieneClave(const char* linea, char clave) {
  return (strchr(linea, clave) != NULL);
}

float GCodeParser::obtenerValor(const char* linea, char clave, float valorDefecto) {
  const char* ptr = strchr(linea, clave);
  if (ptr) {
    return atof(ptr + 1);
  }
  return valorDefecto;
}

void GCodeParser::procesarComando(char* linea, float pasosPorCmX, float pasosPorCmY, CalibracionCallback fnCalibrar) {
  limpiarLinea(linea);

  if (strlen(linea) == 0) return;

  // --- COMANDOS M ---
  if (tieneClave(linea, 'M')) {
    int mVal = (int)obtenerValor(linea, 'M', -1);
    
    if (mVal == 3 || mVal == 30) {
      // M3 / M03: Bajar Lápiz (Encender Spindle)
      servoZ.mover(SERVO_ABAJO);
    } else if (mVal == 5) {
      // M5 / M05: Levantar Lápiz (Apagar Spindle)
      servoZ.mover(SERVO_ARRIBA);
    }
  }

  // --- COMANDOS G ---
  if (tieneClave(linea, 'G')) {
    int gVal = (int)obtenerValor(linea, 'G', -1);

    if (gVal == 90) {
      // G90: Modo Coordenadas Absolutas
      modoAbsoluto = true;
    } else if (gVal == 91) {
      // G91: Modo Coordenadas Relativas
      modoAbsoluto = false;
    } else if (gVal == 28) {
      // G28: Autocalibración u Origen
      if (fnCalibrar != NULL) {
        fnCalibrar();
      }
      setPosicionActualCm(0.0f, 0.0f);
    } else if (gVal == 0 || gVal == 1) {
      // G0 (Posicionamiento rápido) o G1 (Trazado lineal)
      if (gVal == 0 && !tieneClave(linea, 'Z')) {
        // En G0 por defecto levanta lápiz a menos que venga especificado Z
        servoZ.mover(SERVO_ARRIBA);
      }

      if (tieneClave(linea, 'Z')) {
        float zVal = obtenerValor(linea, 'Z', 1.0f);
        if (zVal <= 0.0f) {
          servoZ.mover(SERVO_ABAJO);
        } else {
          servoZ.mover(SERVO_ARRIBA);
        }
      }

      float nuevoX = posXActualCm;
      float nuevoY = posYActualCm;

      if (tieneClave(linea, 'X')) {
        float valX = obtenerValor(linea, 'X', 0.0f);
        nuevoX = modoAbsoluto ? valX : (posXActualCm + valX);
      }

      if (tieneClave(linea, 'Y')) {
        float valY = obtenerValor(linea, 'Y', 0.0f);
        nuevoY = modoAbsoluto ? valY : (posYActualCm + valY);
      }

      long pasosTargetX = (long)(nuevoX * pasosPorCmX);
      long pasosTargetY = (long)(nuevoY * pasosPorCmY);

      planner.iniciarSegmento(pasosTargetX, pasosTargetY);

      // Ejecución coordinada del segmento
      while (!planner.update(DELAY_INTERPOLADO_US)) {
        servoZ.update();
      }

      posXActualCm = nuevoX;
      posYActualCm = nuevoY;
    }
  }
}

void GCodeParser::escucharSerial(float pasosPorCmX, float pasosPorCmY, CalibracionCallback fnCalibrar) {
  while (Serial.available() > 0) {
    char c = Serial.read();

    if (c == '\n' || c == '\r') {
      rxBuffer[rxIndex] = '\0';

      if (rxIndex > 0) {
        procesarComando(rxBuffer, pasosPorCmX, pasosPorCmY, fnCalibrar);
        Serial.println(F("ok"));
      }

      rxIndex = 0;
    } else {
      if (rxIndex < (int)(sizeof(rxBuffer) - 1)) {
        rxBuffer[rxIndex++] = c;
      }
    }
  }
}
