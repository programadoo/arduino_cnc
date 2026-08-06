# Documentación del Proyecto: Ploteador PCB / CNC Arduino Mega 2560

Este proyecto implementa un ploteador CNC de 2 ejes (X, Y) con elevador de lápiz/marcador en el eje Z mediante servomotor. El firmware está optimizado para **Arduino Mega 2560** mediante una arquitectura modular orientada a objetos en C++, 100% no bloqueante, sincronizada en microsegundos y con soporte para **streaming de comandos G-Code en tiempo real vía Puerto Serie (115200 baudios)**.

---

## 📁 Estructura de Archivos del Proyecto

```text
arduino/
├── Config.h            # Configuración global, pines, parámetros mecánicos y enums
├── Config.cpp          # Definición única de la matriz de pasos bipolares (Regla ODR)
├── InputUtils.h        # Clases de periféricos (ServoZ y SwitchInput con debounce)
├── StepperMotor.h      # Interfaz de la clase de control de motores paso a paso
├── StepperMotor.cpp    # Implementación cinemática y bobinado de motores (4 fases bipolares)
├── MotionPlanner.h     # Interfaz del planificador de trayectorias (Bresenham)
├── MotionPlanner.cpp   # Algoritmo de interpolación lineal 2D no bloqueante
├── GCodeParser.h       # Interfaz del parser de comandos G-Code y buffer serie
├── GCodeParser.cpp     # Intérprete de comandos G-Code (G0, G1, G28, M3, M5, G90, G91)
├── sketch.ino          # Archivo principal de Arduino (Setup, Loop no bloqueante y Autoprueba)
├── test_runner.cpp     # Suite de pruebas unitarias nativas en C++
├── build_mega.ps1      # Script de compilación independiente AVR-GCC -> build/sketch.ino.elf
├── diagram.json        # Diagrama circuital interactivo para el simulador Wokwi
├── wokwi.toml          # Configuración del entorno de simulación Wokwi
└── README.md           # Documentación completa del proyecto
```

---

## 🛠️ Descripción Módulo por Módulo

### 1. `Config.h` y `Config.cpp`
Centraliza la configuración de hardware, tiempos, resoluciones y definiciones del sistema.

* **Pines Asignados (Arduino Mega 2560):**
  * **Motor Y (Polea):** Pines `22` (`A+`), `2` (`A-`), `3` (`B+`), `4` (`B-`).
  * **Motor X (Husillo):** Pines `5` (`A+`), `6` (`A-`), `7` (`B+`), `8` (`B-`).
  * **Servomotor Z:** Pin `13` (PWM).
  * **Pulsador Principal SW1:** Pin `30` (Start/Stop).
  * **Switches Fin de Carrera:** `PIN_SW2` (Pin 9), `PIN_SW3` (Pin 10), `PIN_SW4` (Pin 11), `PIN_SW5` (Pin 12).
* **Parámetros Mecánicos y Tiempos:**
  * `PASOS_POR_CM_DEFAULT_X` / `Y` = `100.0f` pasos/cm (Respuesta ágil).
  * `DELAY_PASO1_US` / `DELAY_PASO2_US` / `DELAY_INTERPOLADO_US` = `2000 µs` (500 pasos/segundo).
* **Secuencia de Pasos Bipolar (`PASO_SECUENCIA[4][4]` en `Config.cpp`):**
  Garantiza pulsos electromagnéticos continuos de 90° por paso sin cortocircuitos de fase tanto en hardware real como en el modelo de motor bipolar de Wokwi.

---

### 2. `InputUtils.h`
Contiene controladores de entrada y salida no bloqueantes.

#### Clase `ServoZ` (Control del Lápiz)
* `ServoZ(int pinServo)`: Constructor con asignación de pin PWM.
* `void begin()`: Inicializa el estado del servo.
* `void mover(int angulo)`: Conecta el servo (`attach`) y envía la posición sin usar `delay()`.
* `void update()`: Tras 250 ms desconecta (`detach`) el servo para evitar vibraciones o calentamiento.

#### Clase `SwitchInput` (Antirrebote Digital)
* `SwitchInput(int pin, unsigned long debounceMs)`: Configura el pin con `INPUT_PULLUP` y filtrado digital de antirrebote (10 ms por defecto).
* `bool update()`: Evalúa la estabilidad de la lectura.
* `bool isPressed() const`: Retorna `true` cuando el switch es activado (`LOW`).

---

### 3. `StepperMotor.h` y `StepperMotor.cpp`
Controla el avance físico de los motores paso a paso.

* `StepperMotor(pin1, pin2, pin3, pin4)`: Asigna los 4 pines de fase.
* `void darPaso(bool sentidoHorario)`: Avanza 1 paso en la secuencia de 4 fases bipolares y actualiza el contador acumulado.
* `void apagar()`: Desenergiza todas las bobinas (`LOW`) para reposo térmico.
* `long getPosicion()` / `void setPosicion(long pos)`: Lectura y ajuste del contador interno de pasos.

---

### 4. `MotionPlanner.h` y `MotionPlanner.cpp`
Ejecuta la cinemática coordinada 2D mediante el **Algoritmo de Interpolación Lineal de Bresenham**.

* `void iniciarSegmento(long objX, long objY)`: Calcula deltas, direcciones y error inicial.
* `bool update(unsigned long delayUs)`: Efectúa pasos sincronizados sin bloquear el hilo principal. Retorna `true` al alcanzar la meta.

---

### 5. `GCodeParser.h` y `GCodeParser.cpp`
Intérprete de comandos G-Code estándar para transmisión por puerto serie.

* **Comandos Soportados:**
  * `G0` / `G1`: Movimiento lineal coordinado X/Y.
  * `G28`: Calibración automática de ejes.
  * `M3`: Bajar marcador (posicionar lápiz sobre papel).
  * `M5`: Levantar marcador (posicionar lápiz libre).
  * `G90`: Modo de coordenadas absolutas.
  * `G91`: Modo de coordenadas relativas (incrementales).

---

### 6. `sketch.ino`
Punto de entrada del firmware con máquina de estados no bloqueante.

* `setup()`: Configura Serial a **115200 baudios**, inicializa periféricos y ejecuta `runAutoTest()`.
* `loop()`:
  * Procesa `gcodeParser.escucharSerial(...)` continuamente.
  * Actualiza periféricos (`sw1..5`, `servoZ`).
  * Ejecuta `procesarCalibracionPaso()` de forma no bloqueante durante `CALIBRANDO_INICIAL`.
  * Ejecuta la interpolación de trazado durante `TRAZANDO_DIBUJO`.

---

## 🧪 Pruebas Unitarias y Compilación

### 1. Ejecutar Suite de Pruebas Nativas (C++)
```bash
g++ -std=c++11 -Imock_inc test_runner.cpp StepperMotor.cpp MotionPlanner.cpp GCodeParser.cpp Config.cpp -o test_runner.exe
.\test_runner.exe
```

### 2. Compilar el Binario AVR para Hardware / Wokwi
```powershell
powershell -ExecutionPolicy Bypass -File .\build_mega.ps1
```
*Genera el binario ejecutable `build/sketch.ino.elf` usando los registros reales del microcontrolador ATmega2560.*

---

## 🔌 Esquema de Conexiones Wokwi (`diagram.json`)

| Componente | Pin Hardware Arduino Mega | Pin Componente Wokwi |
|---|---|---|
| **Motor Eje Y** | Pin 22, Pin 2, Pin 3, Pin 4 | `A+`, `A-`, `B+`, `B-` |
| **Motor Eje X** | Pin 5, Pin 6, Pin 7, Pin 8 | `A+`, `A-`, `B+`, `B-` |
| **Servo Eje Z** | Pin 13, 5V, GND | `PWM`, `V+`, `GND` |
| **SW1 (Start/Stop)** | Pin 30, GND | `1.l`, `2.l` |
| **SW2 (Y Límite A)** | Pin 9, GND | `1.l`, `2.l` |
| **SW3 (Y Límite B)** | Pin 10, GND | `1.l`, `2.l` |
| **SW4 (X Límite A)** | Pin 11, GND | `1.l`, `2.l` |
| **SW5 (X Límite B)** | Pin 12, GND | `1.l`, `2.l` |
