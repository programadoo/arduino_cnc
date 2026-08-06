#include "Config.h"

// Secuencia de 4 fases para motor bipolar Wokwi (A+, A-, B+, B-)
// Cada fase energiza UNA bobina con polaridad definida para generar rotación de 90° por paso
// Fase 0: Bobina A forward  (A+=HIGH, A-=LOW)
// Fase 1: Bobina B forward  (B+=HIGH, B-=LOW)
// Fase 2: Bobina A reverse  (A+=LOW,  A-=HIGH)
// Fase 3: Bobina B reverse  (B+=LOW,  B-=HIGH)
const int PASO_SECUENCIA[4][4] = {
  //  A+     A-     B+     B-
  {HIGH,  LOW,   LOW,   LOW },   // Fase 0: Bobina A forward
  {LOW,   LOW,   HIGH,  LOW },   // Fase 1: Bobina B forward
  {LOW,   HIGH,  LOW,   LOW },   // Fase 2: Bobina A reverse
  {LOW,   LOW,   LOW,   HIGH}    // Fase 3: Bobina B reverse
};
