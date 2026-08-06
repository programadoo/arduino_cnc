# Documentación del Proyecto: Ploteador PCB / CNC Arduino Mega

Este proyecto implementa un ploteador CNC de 2 ejes (X, Y) con elevador de lápiz/marcador en el eje Z mediante servomotor. El firmware está optimizado para **Arduino Mega 2560** mediante una arquitectura modular en C++ orientada a objetos, no bloqueante, sincronizada en microsegundos y con soporte para **streaming de comandos G-Code vía Puerto Serie**.

---

## 📁 Estructura de Archivos del Proyecto

```text
c:\Users\HP\Desktop\arduino\
├── Config.h            # Configuración global, pines, parámetros mecánicos y enums
├── InputUtils.h        # Clases de periféricos (ServoZ y SwitchInput con debounce)
├── StepperMotor.h      # Interfaz de la clase de control de motores paso a paso
├── StepperMotor.cpp    # Implementación de la secuencia de pasos de motores
├── MotionPlanner.h     # Interfaz del planificador de trayectorias (Bresenham)
├── MotionPlanner.cpp   # Implementación del algoritmo de interpolación lineal 2D
├── GCodeParser.h       # Interfaz del parser de comandos G-Code y buffer serie
├── GCodeParser.cpp     # Intérprete de comandos G-Code (G0, G1, G28, M3, M5)
├── cnc.ino             # Archivo principal de Arduino (Setup, Loop y Estado Global)
└── README.md           # Documentación del proyecto (este archivo)
```

---

## 🛠️ Descripción de Módulos y Funciones

### 1. `Config.h`
Centraliza todas las constantes del hardware, parámetros de calibración, temporizaciones y definiciones del sistema.

* **Constantes Clave:**
  * Pines de motores (`PASO1_IN1..4`, `PASO2_IN1..4`) y switches (`PIN_SW1..5`).
  * Tiempos de paso (`DELAY_PASO1_US`, `DELAY_PASO2_US`, `DELAY_INTERPOLADO_US`).
  * Dimensiones físicas y offsets (`DISTANCIA_REAL_X/Y_CM`, `OFFSET_ORIGEN_X/Y_CM`).
* **Estructuras y Enums:**
  * `enum EstadoGlobal`: Estados principales del sistema (`SISTEMA_REPOSO`, `CALIBRANDO_INICIAL`, `TRAZANDO_DIBUJO`, `SISTEMA_ERROR`).
  * `enum SubEstadoCalib`: Sub-estados de la calibración de cada eje.
  * `struct Punto`: Define coordenadas `(x, y)` en cm y el estado del lápiz (`bajarLapiz`).
  * `PASO_SECUENCIA[8][4]`: Matriz de medio paso (8 fases) para drivers ULN2003 / 28BYJ-48.

---

### 2. `InputUtils.h`
Contiene clases de utilidad no bloqueantes para el control de insumos y sensores.

#### Clase `ServoZ` (Control del Lápiz)
* `ServoZ(int pinServo)`: Constructor con asignación del pin del servo.
* `void begin()`: Inicialización de variables de posición.
* `void mover(int angulo)`: Conecta el servo y envía el pulso sin usar `delay()`.
* `void update()`: Verifica el tiempo transcurrido (`TIEMPO_SERVO_MS`) y desconecta (`detach()`) el servo para evitar ruidos de retención.
* `int getAnguloActual() const`: Retorna el ángulo actual configurado.
* `bool estaEnMovimiento() const`: Indica si el servo está en transición de posición.

#### Clase `SwitchInput` (Filtrado de Antirrebote)
* `SwitchInput(int pin, unsigned long debounceMs)`: Constructor con pin y ventana de tiempo de antirrebote (por defecto 35ms).
* `void begin()`: Configura el pin como `INPUT_PULLUP` y obtiene la lectura inicial.
* `bool update()`: Lee el pin y confirma cambios solo si la señal se mantiene estable por más del tiempo de debounce.
* `bool isPressed() const`: Retorna `true` si la entrada está activa (`LOW`).
* `int getEstado() const`: Retorna el estado estable del switch (`HIGH` / `LOW`).

---

### 3. `StepperMotor.h` y `StepperMotor.cpp`
Abstrae el comportamiento físico de un motor paso a paso unipolar/bipolar accionado por fases.

* `StepperMotor(int pin1, int pin2, int pin3, int pin4)`: Constructor con los 4 pines de bobinas.
* `void begin()`: Configura los pines como salidas y los desenergiza.
* `void darPaso(bool sentidoHorario)`: Avanza un paso en la secuencia de 8 fases e incrementa/decrementa la posición contada en pasos.
* `void apagar()`: Pone los 4 pines en `LOW` para cortar el consumo eléctrico y prevenir calentamiento en reposo.
* `long getPosicion() const`: Retorna la posición contada acumulada en pasos.
* `void setPosicion(long pos)`: Restablece o establece arbitrariamente el contador de posición en pasos.

---

### 4. `MotionPlanner.h` y `MotionPlanner.cpp`
Gestiona la cinemática de interpolación lineal coordinada de 2 ejes mediante el algoritmo de **Bresenham** no bloqueante.

* `MotionPlanner(StepperMotor& mx, StepperMotor& my)`: Asocia las referencias de los motores X e Y.
* `void iniciarSegmento(long objX, long objY)`: Calcula deltas (`dx`, `dy`), sentido de dirección y el error inicial de Bresenham.
* `bool update(unsigned long delayUs)`: Ejecuta los pasos sincronizados según el temporizador `micros()`. Retorna `true` cuando ambos ejes alcanzan la posición objetivo.
* `bool isSegmentoIniciado() const`: Retorna si hay un segmento activo en ejecución.
* `void resetSegmento()`: Reinicia el estado del planificador para un nuevo tramo.

---

### 5. `GCodeParser.h` y `GCodeParser.cpp`
Receptor e intérprete de comandos G-Code estándar para streaming en tiempo real vía USB.

* `GCodeParser(MotionPlanner& mp, ServoZ& sz)`: Constructor asociado a las instancias del planificador y del servo.
* `void escucharSerial(float pasosPorCmX, float pasosPorCmY, CalibracionCallback fnCalibrar)`: Escucha la consola serie sin bloquear el CPU. Al recibir `\n` ejecuta el comando y responde `ok`.
* `void procesarComando(...)`: Tokeniza y ejecuta instrucciones:
  * `G0` / `G00`: Posicionamiento rápido.
  * `G1` / `G01`: Movimiento lineal coordinado X/Y.
  * `G28`: Autocalibración de ejes.
  * `M3` / `M03`: Bajar lápiz (servomotor a `SERVO_ABAJO`).
  * `M5` / `M05`: Levantar lápiz (servomotor a `SERVO_ARRIBA`).
  * `G90` / `G91`: Conmutación entre coordenadas absolutas y relativas.

---

### 6. `cnc.ino`
Archivo de entrada de Arduino. Implementa la máquina de estados principal.

* `setup()`: Inicializa el puerto Serie a 9600 baudios, arranca periféricos y posiciona el servo levantado.
* `loop()`: 
  1. Escucha continuamente comandos G-Code vía `gcodeParser.escucharSerial(...)`.
  2. Actualiza periódicamente `servoZ` y los 5 switches (`SW1` a `SW5`).
  3. Detecta clics en `SW1` (clic simple = inicio calibración / parada de emergencia; doble clic = trazado directo).
  4. Ejecuta el ciclo de calibración de ejes X e Y (`CALIBRANDO_INICIAL`) registrando distancias físicas y aplicando offsets.
  5. Ejecuta el trazado estático o dinámico llamando al planificador.
* `detencionDeEmergencia()`: Apaga motores, levanta lápiz y pasa el sistema a `SISTEMA_REPOSO`.

---

## ⚠️ VALORES CRÍTICOS A REVISAR ANTES DE CARGAR AL ARDUINO MEGA

Antes de compilar y subir el código al **Arduino Mega 2560**, debes revisar cuidadosamente los siguientes parámetros en **[Config.h](file:///c:/Users/HP/Desktop/arduino/Config.h)**:

### 1. Asignación Física de Pines
```cpp
const int PIN_SW1 = 30;   // Botón Start / Stop (Requiere Pull-up interno)

// Motor Y (Polea)
const int PASO1_IN1 = 22; // Evitar Pines 0 y 1 (Usados por Serial USB)
const int PASO1_IN2 = 2;
const int PASO1_IN3 = 3;
const int PASO1_IN4 = 4;

// Motor X (Husillo)
const int PASO2_IN1 = 5;
const int PASO2_IN2 = 6;
const int PASO2_IN3 = 7;
const int PASO2_IN4 = 8;

// Fin de Carrera
const int PIN_SW2 = 9;   // Límite A (Y)
const int PIN_SW3 = 10;  // Límite B (Y)
const int PIN_SW4 = 11;  // Límite A (X)
const int PIN_SW5 = 12;  // Límite B (X)

const int PIN_SERVO = 13; // Pin PWM del Servo Z
```

---

### 2. Ángulos de Recorrido del Servomotor Z
```cpp
const int SERVO_ABAJO = 55;    // Ángulo en grados donde el marcador toca la PCB/Papel
const int SERVO_ARRIBA = 115;  // Ángulo en grados donde el marcador queda libre en el aire
```

---

### 3. Delays de Velocidad por Paso (`micros`)
```cpp
const unsigned long DELAY_PASO1_US = 6000;  // Delay Eje Y
const unsigned long DELAY_PASO2_US = 1300;  // Delay Eje X
const unsigned long DELAY_INTERPOLADO_US = 1400; // Delay base para trazado vectorial
```

---

### 4. Distancias Físicas y Offsets Mecánicos
```cpp
const float DISTANCIA_REAL_X_CM = 28.3f; // Distancia en cm desde Switch SW4 a SW5
const float DISTANCIA_REAL_Y_CM = 21.5f; // Distancia en cm desde Switch SW2 a SW3

const float OFFSET_ORIGEN_X_CM = 1.9f;  // Distancia del centro físico al (0,0) de trabajo
const float OFFSET_ORIGEN_Y_CM = 2.8f;  
```
