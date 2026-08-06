#include "Config.h"

// Definición única de la secuencia de 4 fases para motores bipolares/unipolares (Wokwi / NEMA / ULN)
// Esta es la ÚNICA definición en todo el proyecto (regla ODR de C++)
const int PASO_SECUENCIA[4][4] = {
  {HIGH, LOW,  LOW,  LOW},
  {LOW,  LOW,  HIGH, LOW},
  {LOW,  HIGH, LOW,  LOW},
  {LOW,  LOW,  LOW,  HIGH}
};
