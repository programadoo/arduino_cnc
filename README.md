# Documentación del Proyecto: Ploteador PCB / CNC Arduino Mega 2560

Este proyecto implementa un ploteador CNC de 2 ejes (X, Y) con elevador de lápiz/marcador en el eje Z mediante servomotor. El firmware está optimizado para **Arduino Mega 2560** mediante una arquitectura modular orientada a objetos en C++, 100% no bloqueante, sincronizada en microsegundos y con soporte para **streaming de comandos G-Code en tiempo real vía Puerto Serie (115200 baudios)**.

---

## 📦 ARCHIVOS ESENCIALES PARA CARGAR AL ARDUINO MEGA REAL

Para compilar y subir el firmware a tu placa física de **Arduino Mega 2560** desde el Arduino IDE o PlatformIO, debes abrir la carpeta que contiene únicamente los siguientes **10 archivos esenciales**:

```text
📁 Proyecto_Arduino_Mega/
├── sketch.ino        # (1) Punto de entrada del firmware (setup, loop y máquina de estados)
├── Config.h          # (2) Asignación de pines del Arduino Mega y constantes de tiempo
├── Config.cpp        # (3) Definición única de la matriz de pasos bipolares (Regla ODR C++)
├── InputUtils.h      # (4) Controladores no bloqueantes del Servo Z y filtrado de switches
├── StepperMotor.h    # (5) Interfaz orientada a objetos para motores paso a paso
├── StepperMotor.cpp  # (6) Manejo físico de bobinas y contadores de pasos de motores
├── MotionPlanner.h   # (7) Interfaz del planificador de trayectorias coordinadas 2D
├── MotionPlanner.cpp # (8) Algoritmo de interpolación lineal no bloqueante (Bresenham)
├── GCodeParser.h     # (9) Interfaz del receptor e intérprete de comandos G-Code
└── GCodeParser.cpp   # (10) Parser serie para instrucciones G0, G1, G28, M3, M5, G90, G91
```

### 💡 ¿Por qué son indispensables estos 10 archivos?
* **`sketch.ino`**: Es el archivo principal que Arduino IDE busca al compilar. Arranca los módulos y ejecuta la máquina de estados.
* **`Config.h` / `Config.cpp`**: Indican al microcontrolador a qué pines digitales enviar los pulsos y evitan errores de múltiples definiciones.
* **`InputUtils.h`**: Permite subir/bajar el lápiz suavemente sin congelar la ejecución y limpia los rebotes de los botones.
* **`StepperMotor.h` / `.cpp`**: Generan la secuencia exacta de pulsos para que los motores giren en ambos sentidos.
* **`MotionPlanner.h` / `.cpp`**: Hacen que los motores X e Y se muevan juntos en diagonal de forma coordinada.
* **`GCodeParser.h` / `.cpp`**: Permiten recibir comandos por USB desde programas como FlatCAM, Universal G-Code Sender o Python.

> [!WARNING]
> **Archivos que NO debes incluir al cargar al Arduino Real:**
> `diagram.json`, `wokwi.toml`, `build_mega.ps1`, `test_runner.cpp`, carpetas `mock_inc/`, `avr_inc/`, `arduino_core/`. Estos archivos corresponden exclusivamente al entorno de simulación en PC y Wokwi.

---

## 🧠 RETORNO AUTOMÁTICO AL ORIGEN (0,0)

El firmware incluye una función inteligente de seguimiento de posición y retorno automático:

1. **Rastreo continuo de coordenadas:** Durante todo el trabajo de trazado, el planificador calcula la posición absoluta `(posXActualCm, posYActualCm)`.
2. **Retorno Automático al Finalizar:** Al completar la última línea de un circuito PCB, el sistema levanta el lápiz (`M5`) y ejecuta automáticamente un desplazamiento de retorno a la coordenada originaria **(0,0)** antes de apagar los motores.
3. **Reinicio Seguro:** Al encender o reiniciar la placa, el firmware asume la posición actual como su referencia de origen `(0,0)`, garantizando que cualquier comando de retorno vuelva de forma segura sin colisionar.

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
