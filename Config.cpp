#include "Config.h"

// Definición única de la secuencia de medio paso (Half-stepping) de 8 fases para ULN2003
// Esta es la ÚNICA definición en todo el proyecto (regla ODR de C++)
const int PASO_SECUENCIA[8][4] = {
  {HIGH, LOW,  LOW,  LOW},
  {HIGH, HIGH, LOW,  LOW},
  {LOW,  HIGH, LOW,  LOW},
  {LOW,  HIGH, HIGH, LOW},
  {LOW,  LOW,  HIGH, LOW},
  {LOW,  LOW,  HIGH, HIGH},
  {LOW,  LOW,  LOW,  HIGH},
  {HIGH, LOW,  LOW,  HIGH}
};
