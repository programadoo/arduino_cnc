#include "Config.h"
#include "StepperMotor.h"
#include "MotionPlanner.h"
#include "InputUtils.h"

// =========================================================
// --- INSTANCIAS GLOBALES ---
// =========================================================

// Motores Paso a Paso
StepperMotor motorY(PASO1_IN1, PASO1_IN2, PASO1_IN3, PASO1_IN4); // Motor 1 (Eje Y)
StepperMotor motorX(PASO2_IN1, PASO2_IN2, PASO2_IN3, PASO2_IN4); // Motor 2 (Eje X)

// Planificador de Movimiento Coordinado
MotionPlanner planner(motorX, motorY);

// Servomotor Z
ServoZ servoZ(PIN_SERVO);

// Sensores y Botón
SwitchInput sw1(PIN_SW1, 50); // Start / Stop
SwitchInput sw2(PIN_SW2);     // Motor Y Límite A
SwitchInput sw3(PIN_SW3);     // Motor Y Límite B
SwitchInput sw4(PIN_SW4);     // Motor X Límite A
SwitchInput sw5(PIN_SW5);     // Motor X Límite B

// --- VECTOR DE COORDENADAS: CIRCUITO COMPLETO ---
const Punto TRAZO_CIRCUITO[] = {
  { 0.0,  0.0, false},
  {-2.1,  0.8, true},
  {-0.4,  0.8, true},
  {-0.4,  1.8, true},
  { 0.0,  1.8, true},
  { 0.0, -0.6, true},
  { 0.4,  1.8, true},
  { 0.4,  0.8, true},
  { 2.1,  0.8, true},
  {-0.4, -2.0, true},
  {-0.4, -0.1, true},
  { 2.1, -0.1, true},
  { 0.4, -1.2, true},
  { 0.4, -2.0, true},
  { 0.0,  0.0, false}
};

const int TOTAL_PUNTOS = sizeof(TRAZO_CIRCUITO) / sizeof(TRAZO_CIRCUITO[0]);

// =========================================================
// --- VARIABLES DE ESTADO Y CONTROL ---
// =========================================================

EstadoGlobal estadoGlobal = SISTEMA_REPOSO;
SubEstadoCalib estadoM1 = CAL_BUSCANDO_SW_A;
SubEstadoCalib estadoM2 = CAL_BUSCANDO_SW_A;

int estadoSW1Anterior = HIGH;
int conteoClicksSW1 = 0;
unsigned long tiempoPrimerClick = 0;
int indicePuntoActual = 0;

// Escala inicializada con resolución predeterminada teórica
float pasosPorCmX = PASOS_POR_CM_DEFAULT_X;
float pasosPorCmY = PASOS_POR_CM_DEFAULT_Y;

// Variables de Calibración Eje Y (Motor 1)
long pasosMedidosM1 = 0;
long pasosObjetivoCentroM1 = 0;
long pasosOffsetM1 = 0;
long pasosRecorridosOffsetM1 = 0;
unsigned long ultimoPaso1Us = 0;
long pasosSeguridadCalibM1 = 0;

// Variables de Calibración Eje X (Motor 2)
long pasosMedidosM2 = 0;
long pasosObjetivoCentroM2 = 0;
long pasosOffsetM2 = 0;
long pasosRecorridosOffsetM2 = 0;
unsigned long ultimoPaso2Us = 0;
long pasosSeguridadCalibM2 = 0;

// =========================================================
// --- FUNCIONES AUXILIARES ---
// =========================================================

void detencionDeEmergencia() {
  motorY.apagar();
  motorX.apagar();
  servoZ.mover(SERVO_ARRIBA);
  estadoGlobal = SISTEMA_REPOSO;
  conteoClicksSW1 = 0;
  planner.resetSegmento();
  Serial.println(F("=== PARADA DE EMERGENCIA / DETENCION ACTIVADA ==="));
}

void iniciarTrazadoDirecto() {
  servoZ.mover(SERVO_ARRIBA);
  motorX.setPosicion(0);
  motorY.setPosicion(0);
  indicePuntoActual = 0;
  planner.resetSegmento();
  estadoGlobal = TRAZANDO_DIBUJO;
  Serial.println(F("--> INICIANDO TRAZADO DIRECTO..."));
}

// =========================================================
// --- ARDUINO SETUP Y LOOP ---
// =========================================================

void setup() {
  Serial.begin(9600);
  Serial.println(F("=== INICIANDO PLOTEADOR PCB (MODULAR) ==="));

  motorY.begin();
  motorX.begin();
  servoZ.begin();

  sw1.begin();
  sw2.begin(); sw3.begin();
  sw4.begin(); sw5.begin();

  servoZ.mover(SERVO_ARRIBA);
}

void loop() {
  unsigned long ahoraMs = millis();
  unsigned long ahoraUs = micros();

  // Actualizar lecturas de entradas y servos de forma no bloqueante
  servoZ.update();
  sw1.update();
  sw2.update(); sw3.update();
  sw4.update(); sw5.update();

  int estadoSW1 = sw1.getEstado();

  // Detección de pulsaciones en SW1
  if (estadoSW1 == LOW && estadoSW1Anterior == HIGH) {
    if (estadoGlobal != SISTEMA_REPOSO) {
      detencionDeEmergencia();
    } else {
      conteoClicksSW1++;
      if (conteoClicksSW1 == 1) {
        tiempoPrimerClick = ahoraMs;
      } else if (conteoClicksSW1 == 2) {
        conteoClicksSW1 = 0;
        iniciarTrazadoDirecto();
      }
    }
  }
  estadoSW1Anterior = estadoSW1;

  // Manejo de clic simple para iniciar calibración tras tiempo límite de doble clic
  if (estadoGlobal == SISTEMA_REPOSO && conteoClicksSW1 == 1) {
    if (ahoraMs - tiempoPrimerClick > TIEMPO_DOBLE_CLICK_MS) {
      conteoClicksSW1 = 0;
      servoZ.mover(SERVO_ARRIBA);
      pasosMedidosM1 = 0; pasosRecorridosOffsetM1 = 0;
      pasosMedidosM2 = 0; pasosRecorridosOffsetM2 = 0;
      motorX.setPosicion(0);
      motorY.setPosicion(0);
      indicePuntoActual = 0;
      planner.resetSegmento();
      
      estadoGlobal = CALIBRANDO_INICIAL;
      estadoM1 = CAL_BUSCANDO_SW_A; ultimoPaso1Us = ahoraUs; pasosSeguridadCalibM1 = 0;
      estadoM2 = CAL_BUSCANDO_SW_A; ultimoPaso2Us = ahoraUs; pasosSeguridadCalibM2 = 0;
      Serial.println(F("--> INICIANDO CALIBRACION AUTOMATICA DE EJES..."));
    }
  }

  if (estadoGlobal == SISTEMA_REPOSO || estadoGlobal == SISTEMA_ERROR) return;

  // =========================================================
  // CALIBRACIÓN INICIAL CON PROTECCIÓN ANTI-BLOQUEO MEJORADA
  // =========================================================
  if (estadoGlobal == CALIBRANDO_INICIAL) {
    
    // --- EJE Y (Motor 1) ---
    switch (estadoM1) {
      case CAL_BUSCANDO_SW_A:
        if (ahoraUs - ultimoPaso1Us >= DELAY_PASO1_US) { 
          ultimoPaso1Us = ahoraUs; 
          motorY.darPaso(true); 
          pasosSeguridadCalibM1++;
        }
        if (sw2.isPressed()) {
          pasosMedidosM1 = 0; 
          pasosSeguridadCalibM1 = 0;
          estadoM1 = CAL_BUSCANDO_SW_B; 
        }
        // Control de Error por Timeout en Calibración
        if (pasosSeguridadCalibM1 > MAX_PASOS_SEGURIDAD) {
          Serial.println(F("!!! ERROR CRITICO: TIMEOUT EN CALIBRACION EJE Y (Switch SW2 no respondio) !!!"));
          detencionDeEmergencia();
          estadoGlobal = SISTEMA_ERROR;
          return;
        }
        break;

      case CAL_BUSCANDO_SW_B:
        if (ahoraUs - ultimoPaso1Us >= DELAY_PASO1_US) { 
          ultimoPaso1Us = ahoraUs; 
          motorY.darPaso(false); 
          pasosMedidosM1++; 
        }
        if (sw3.isPressed() && pasosMedidosM1 > 200) {
          pasosPorCmY = (float)pasosMedidosM1 / DISTANCIA_REAL_Y_CM;
          pasosObjetivoCentroM1 = pasosMedidosM1 / 2;
          pasosOffsetM1 = (long)(OFFSET_ORIGEN_Y_CM * pasosPorCmY);
          
          Serial.print(F("--> CALIBRACION EJE Y COMPLETADA: Pasos Totales = "));
          Serial.print(pasosMedidosM1);
          Serial.print(F(" | Pasos/cm = "));
          Serial.println(pasosPorCmY, 2);

          estadoM1 = CAL_MOVIENDO_CENTRO; 
        }
        break;

      case CAL_MOVIENDO_CENTRO:
        if (ahoraUs - ultimoPaso1Us >= DELAY_PASO1_US) {
          ultimoPaso1Us = ahoraUs;
          if (pasosMedidosM1 > pasosObjetivoCentroM1) {
            motorY.darPaso(true);
            pasosMedidosM1--;
          } else {
            estadoM1 = CAL_APLICANDO_OFFSET;
          }
        }
        break;

      case CAL_APLICANDO_OFFSET:
        if (ahoraUs - ultimoPaso1Us >= DELAY_PASO1_US) {
          ultimoPaso1Us = ahoraUs;
          if (pasosRecorridosOffsetM1 < abs(pasosOffsetM1)) {
            motorY.darPaso(pasosOffsetM1 > 0);
            pasosRecorridosOffsetM1++;
          } else {
            estadoM1 = CAL_LISTO;
          }
        }
        break;

      case CAL_LISTO:
      case CAL_ERROR:
        break;
    }

    // --- EJE X (Motor 2) ---
    switch (estadoM2) {
      case CAL_BUSCANDO_SW_A:
        if (ahoraUs - ultimoPaso2Us >= DELAY_PASO2_US) { 
          ultimoPaso2Us = ahoraUs; 
          motorX.darPaso(true); 
          pasosSeguridadCalibM2++;
        }
        if (sw4.isPressed()) {
          pasosMedidosM2 = 0; 
          pasosSeguridadCalibM2 = 0;
          estadoM2 = CAL_BUSCANDO_SW_B; 
        }
        // Control de Error por Timeout en Calibración
        if (pasosSeguridadCalibM2 > MAX_PASOS_SEGURIDAD) {
          Serial.println(F("!!! ERROR CRITICO: TIMEOUT EN CALIBRACION EJE X (Switch SW4 no respondio) !!!"));
          detencionDeEmergencia();
          estadoGlobal = SISTEMA_ERROR;
          return;
        }
        break;

      case CAL_BUSCANDO_SW_B:
        if (ahoraUs - ultimoPaso2Us >= DELAY_PASO2_US) { 
          ultimoPaso2Us = ahoraUs; 
          motorX.darPaso(false); 
          pasosMedidosM2++; 
        }
        if (sw5.isPressed() && pasosMedidosM2 > 1500) {
          pasosPorCmX = (float)pasosMedidosM2 / DISTANCIA_REAL_X_CM;
          pasosObjetivoCentroM2 = pasosMedidosM2 / 2;
          pasosOffsetM2 = (long)(OFFSET_ORIGEN_X_CM * pasosPorCmX);
          
          Serial.print(F("--> CALIBRACION EJE X COMPLETADA: Pasos Totales = "));
          Serial.print(pasosMedidosM2);
          Serial.print(F(" | Pasos/cm = "));
          Serial.println(pasosPorCmX, 2);

          estadoM2 = CAL_MOVIENDO_CENTRO; 
        }
        break;

      case CAL_MOVIENDO_CENTRO:
        if (ahoraUs - ultimoPaso2Us >= DELAY_PASO2_US) {
          ultimoPaso2Us = ahoraUs;
          if (pasosMedidosM2 > pasosObjetivoCentroM2) {
            motorX.darPaso(true);
            pasosMedidosM2--;
          } else {
            estadoM2 = CAL_APLICANDO_OFFSET;
          }
        }
        break;

      case CAL_APLICANDO_OFFSET:
        if (ahoraUs - ultimoPaso2Us >= DELAY_PASO2_US) {
          ultimoPaso2Us = ahoraUs;
          if (pasosRecorridosOffsetM2 < abs(pasosOffsetM2)) {
            motorX.darPaso(pasosOffsetM2 > 0);
            pasosRecorridosOffsetM2++;
          } else {
            estadoM2 = CAL_LISTO;
          }
        }
        break;

      case CAL_LISTO:
      case CAL_ERROR:
        break;
    }

    // Si ambos ejes terminaron exitosamente su calibración
    if (estadoM1 == CAL_LISTO && estadoM2 == CAL_LISTO) {
      motorY.apagar();
      motorX.apagar();
      motorX.setPosicion(0);
      motorY.setPosicion(0);
      
      Serial.println(F("=== CALIBRACION EXITOSA: POSICIONADO EN ORIGEN (0,0) ==="));
      Serial.print(F("Origen X Offset Pasos: ")); Serial.println(pasosOffsetM2);
      Serial.print(F("Origen Y Offset Pasos: ")); Serial.println(pasosOffsetM1);
      
      delay(500);
      estadoGlobal = TRAZANDO_DIBUJO;
      indicePuntoActual = 0;
      planner.resetSegmento();
    }
    return;
  }

  // =========================================================
  // EJECUCIÓN DEL TRAZADO DEL CIRCUITO (PLANIFICADOR BRESENHAM)
  // =========================================================
  if (estadoGlobal == TRAZANDO_DIBUJO) {
    if (indicePuntoActual >= TOTAL_PUNTOS) {
      motorY.apagar();
      motorX.apagar();
      servoZ.mover(SERVO_ARRIBA);
      estadoGlobal = SISTEMA_REPOSO;
      Serial.println(F("=== TRAZADO COMPLETADO CON EXITO ==="));
      return;
    }

    Punto pt = TRAZO_CIRCUITO[indicePuntoActual];

    if (pt.bajarLapiz) {
      servoZ.mover(SERVO_ABAJO);
    } else {
      servoZ.mover(SERVO_ARRIBA);
    }

    long posXObjetivoPasos = (long)(pt.x * pasosPorCmX);
    long posYObjetivoPasos = (long)(pt.y * pasosPorCmY);

    if (!planner.isSegmentoIniciado()) {
      planner.iniciarSegmento(posXObjetivoPasos, posYObjetivoPasos);
    }

    bool llegado = planner.update(DELAY_INTERPOLADO_US);

    if (llegado) {
      Serial.print(F("Punto "));
      Serial.print(indicePuntoActual);
      Serial.print(F(" Alcanzado | cm ("));
      Serial.print(pt.x, 2); Serial.print(F(", "));
      Serial.print(pt.y, 2);
      Serial.print(F(") -> Pasos ("));
      Serial.print(motorX.getPosicion()); Serial.print(F(", "));
      Serial.print(motorY.getPosicion());
      Serial.println(F(")"));

      indicePuntoActual++;
    }
  }
}