#include <iostream>
#include <cassert>
#include <cmath>

#include "Config.h"
#include "StepperMotor.h"
#include "MotionPlanner.h"
#include "InputUtils.h"
#include "GCodeParser.h"

#define RUN_TEST(test_func) \
  do { \
    std::cout << "[RUNNING] " << #test_func << "... "; \
    test_func(); \
    std::cout << "PASSED" << std::endl; \
  } while(0)

void test_stepper_motor() {
  StepperMotor motor(22, 2, 3, 4);
  motor.begin();

  assert(motor.getPosicion() == 0);

  // Dar 10 pasos en sentido horario
  for (int i = 0; i < 10; i++) {
    motor.darPaso(true);
  }
  assert(motor.getPosicion() == 10);

  // Dar 4 pasos en sentido antihorario
  for (int i = 0; i < 4; i++) {
    motor.darPaso(false);
  }
  assert(motor.getPosicion() == 6);

  motor.apagar();
}

void test_motion_planner_bresenham() {
  StepperMotor motorX(5, 6, 7, 8);
  StepperMotor motorY(22, 2, 3, 4);
  motorX.begin();
  motorY.begin();

  MotionPlanner planner(motorX, motorY);

  // Prueba 1: Movimiento Horizontal (X=100, Y=0)
  planner.iniciarSegmento(100, 0);
  while (!planner.update(100)) {
    delayMicroseconds(100);
  }

  assert(motorX.getPosicion() == 100);
  assert(motorY.getPosicion() == 0);

  // Prueba 2: Movimiento Coordinado (X=200, Y=50)
  planner.iniciarSegmento(200, 50);
  while (!planner.update(100)) {
    delayMicroseconds(100);
  }

  assert(motorX.getPosicion() == 200);
  assert(motorY.getPosicion() == 50);

  motorX.apagar();
  motorY.apagar();
}

void test_gcode_parser() {
  StepperMotor motorX(5, 6, 7, 8);
  StepperMotor motorY(22, 2, 3, 4);
  MotionPlanner planner(motorX, motorY);
  ServoZ servoZ(13);

  GCodeParser parser(planner, servoZ);
  parser.begin();

  // Test M3 (Bajar Lápiz)
  char cmdM3[] = "M3";
  parser.procesarComando(cmdM3, 100.0f, 100.0f, NULL);
  // Avanzar tiempo para actualizar servo
  delay(300);
  servoZ.update();
  assert(servoZ.getAnguloActual() == SERVO_ABAJO);

  // Test M5 (Levantar Lápiz)
  char cmdM5[] = "M5";
  parser.procesarComando(cmdM5, 100.0f, 100.0f, NULL);
  delay(300);
  servoZ.update();
  assert(servoZ.getAnguloActual() == SERVO_ARRIBA);

  // Test G90 / G91 (Modo Absoluto vs Relativo)
  char cmdG90[] = "G90";
  parser.procesarComando(cmdG90, 100.0f, 100.0f, NULL);
  assert(parser.isModoAbsoluto() == true);

  char cmdG91[] = "G91";
  parser.procesarComando(cmdG91, 100.0f, 100.0f, NULL);
  assert(parser.isModoAbsoluto() == false);

  // Test G1 Movimiento Linear
  char cmdG90Reset[] = "G90";
  parser.procesarComando(cmdG90Reset, 10.0f, 10.0f, NULL);

  char cmdG1[] = "G1 X2 Y3";
  parser.procesarComando(cmdG1, 10.0f, 10.0f, NULL); // 10 pasos/cm

  assert(parser.getPosXActualCm() == 2.0f);
  assert(parser.getPosYActualCm() == 3.0f);
  assert(motorX.getPosicion() == 20); // 2 cm * 10 pasos/cm
  assert(motorY.getPosicion() == 30); // 3 cm * 10 pasos/cm
}

int main() {
  std::cout << "============================================" << std::endl;
  std::cout << "   SUITE DE PRUEBAS UNITARIAS C++ NATIVO    " << std::endl;
  std::cout << "============================================" << std::endl;

  RUN_TEST(test_stepper_motor);
  RUN_TEST(test_motion_planner_bresenham);
  RUN_TEST(test_gcode_parser);

  std::cout << "============================================" << std::endl;
  std::cout << "   TODAS LAS PRUEBAS UNITARIAS PASARON (100% OK)" << std::endl;
  std::cout << "============================================" << std::endl;

  return 0;
}
