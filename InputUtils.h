#ifndef INPUT_UTILS_H
#define INPUT_UTILS_H

#include "Config.h"
#include <Servo.h>

// Control No Bloqueante del Servomotor (Eje Z)
class ServoZ {
private:
  Servo miServo;
  int pin;
  int anguloActual;
  int anguloObjetivo;
  unsigned long tiempoInicioMovimiento;
  bool enMovimiento;

public:
  ServoZ(int pinServo)
    : pin(pinServo), anguloActual(-1), anguloObjetivo(-1),
      tiempoInicioMovimiento(0), enMovimiento(false) {}

  void begin() {
    anguloActual = -1;
    anguloObjetivo = -1;
    enMovimiento = false;
  }

  void mover(int angulo) {
    if (anguloActual != angulo && anguloObjetivo != angulo) {
      miServo.attach(pin);
      miServo.write(angulo);
      anguloObjetivo = angulo;
      tiempoInicioMovimiento = millis();
      enMovimiento = true;
    }
  }

  void update() {
    if (enMovimiento && (millis() - tiempoInicioMovimiento >= TIEMPO_SERVO_MS)) {
      miServo.detach();
      anguloActual = anguloObjetivo;
      enMovimiento = false;
    }
  }

  int getAnguloActual() const {
    return anguloActual;
  }

  bool estaEnMovimiento() const {
    return enMovimiento;
  }
};

// Antirrebote (Debounce) No Bloqueante para Switches / Botones
class SwitchInput {
private:
  int pin;
  int estadoEstable;
  int ultimoEstadoLectura;
  unsigned long tiempoUltimoCambio;
  unsigned long tiempoDebounce;

public:
  SwitchInput(int p, unsigned long debounceMs = TIEMPO_ANTIRREBOTE)
    : pin(p), estadoEstable(HIGH), ultimoEstadoLectura(HIGH),
      tiempoUltimoCambio(0), tiempoDebounce(debounceMs) {}

  void begin() {
    pinMode(pin, INPUT_PULLUP);
    estadoEstable = digitalRead(pin);
    ultimoEstadoLectura = estadoEstable;
  }

  bool update() {
    int lecturaActual = digitalRead(pin);
    unsigned long ahora = millis();

    if (lecturaActual != ultimoEstadoLectura) {
      tiempoUltimoCambio = ahora;
      ultimoEstadoLectura = lecturaActual;
    }

    if ((ahora - tiempoUltimoCambio) > tiempoDebounce) {
      if (lecturaActual != estadoEstable) {
        estadoEstable = lecturaActual;
      }
    }
    return estadoEstable;
  }

  bool isPressed() const {
    return estadoEstable == LOW;
  }

  int getEstado() const {
    return estadoEstable;
  }
};

#endif // INPUT_UTILS_H
