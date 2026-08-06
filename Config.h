#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// =========================================================
// --- CONFIGURACIÓN Y ASIGNACIÓN DE PINES ---
// =========================================================

// Botón Principal
const int PIN_SW1 = 30;  // Botón Start / Stop (Pin 30)

// Motor Paso a Paso 1 (Eje Y - Polea)
const int PASO1_IN1 = 22; 
const int PASO1_IN2 = 2;
const int PASO1_IN3 = 3;
const int PASO1_IN4 = 4;

// Motor Paso a Paso 2 (Eje X - Husillo/Rosca)
const int PASO2_IN1 = 5;
const int PASO2_IN2 = 6;
const int PASO2_IN3 = 7;
const int PASO2_IN4 = 8;

// Fin de Carrera (Switches)
const int PIN_SW2 = 9;   // Motor 1 (Límite A - Eje Y)
const int PIN_SW3 = 10;  // Motor 1 (Límite B - Eje Y)
const int PIN_SW4 = 11;  // Motor 2 (Límite A - Eje X)
const int PIN_SW5 = 12;  // Motor 2 (Límite B - Eje X)

// Servomotor (Eje Z / Marcador)
const int PIN_SERVO = 13;

// =========================================================
// --- PARÁMETROS MECÁNICOS Y DE CALIBRACIÓN ---
// =========================================================

// Posiciones del Servo (Z)
const int SERVO_ABAJO = 55;     // Marcador apoyado (trazando)
const int SERVO_ARRIBA = 115;   // Marcador levantado (desplazamiento)
const unsigned long TIEMPO_SERVO_MS = 250; // Tiempo no bloqueante para movimiento del servo

// Velocidades de Calibración (Delays en microsegundos)
const unsigned long DELAY_PASO1_US = 6000;  // Eje Y
const unsigned long DELAY_PASO2_US = 1300;  // Eje X

// Delay base para el trazado vector sincronizado (Interpolación Bresenham)
const unsigned long DELAY_INTERPOLADO_US = 1400; 

// Distancias Físicas de Calibración (Mínimo a Máximo Switch)
const float DISTANCIA_REAL_X_CM = 28.3f; 
const float DISTANCIA_REAL_Y_CM = 21.5f; 

// Offsets de Origen (Desde el centro físico al (0,0) de trabajo)
const float OFFSET_ORIGEN_X_CM = 1.9f;  
const float OFFSET_ORIGEN_Y_CM = 2.8f;  

// Resolución por defecto (Teórica) para evitar divisiones por cero antes de calibrar
const float PASOS_POR_CM_DEFAULT_X = 1473.5f; // ~41700 pasos / 28.3 cm
const float PASOS_POR_CM_DEFAULT_Y = 223.25f;  // ~4800 pasos / 21.5 cm

// Parámetros de temporizaciones
const unsigned long TIEMPO_DOBLE_CLICK_MS = 400; 
const unsigned long TIEMPO_ANTIRREBOTE = 10;
const long MAX_PASOS_SEGURIDAD = 60000; // Pasos máximos de seguridad para calibración

// =========================================================
// --- ESTRUCTURAS Y ESTADOS GLOBALES ---
// =========================================================

enum EstadoGlobal { 
  SISTEMA_REPOSO, 
  CALIBRANDO_INICIAL, 
  TRAZANDO_DIBUJO, 
  SISTEMA_ERROR 
};

enum SubEstadoCalib { 
  CAL_BUSCANDO_SW_A, 
  CAL_BUSCANDO_SW_B, 
  CAL_MOVIENDO_CENTRO, 
  CAL_APLICANDO_OFFSET, 
  CAL_LISTO, 
  CAL_ERROR 
};

struct Punto {
  float x; 
  float y; 
  bool bajarLapiz; 
};

// Secuencia de 4 fases para motores bipolares (Wokwi / NEMA 17 / ULN2003)
// NOTA: La definición real vive en Config.cpp (una sola definición, regla ODR de C++)
extern const int PASO_SECUENCIA[4][4];

#endif // CONFIG_H
