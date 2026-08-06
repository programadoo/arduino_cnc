$env:PATH = "$PSScriptRoot\tools\avr-gcc-14.1.0-x64-windows\bin;$PSScriptRoot\tools\avr-gcc-14.1.0-x64-windows\libexec\gcc\avr\14.1.0;" + $env:PATH
if (-not (Test-Path "$PSScriptRoot\build")) { New-Item -ItemType Directory -Path "$PSScriptRoot\build" }

Write-Host "Compilando ARDUINO CORE OFICIAL para ATmega2560..." -ForegroundColor Cyan

$FLAGS = "-mmcu=atmega2560 -DF_CPU=16000000UL -DARDUINO=10819 -DARDUINO_AVR_MEGA2560 -DARDUINO_ARCH_AVR -Os -Iarduino_core -I."

# 1. Compilar C de Arduino Core
avr-gcc $FLAGS.Split(' ') -c arduino_core/wiring.c -o build/wiring.o
avr-gcc $FLAGS.Split(' ') -c arduino_core/wiring_digital.c -o build/wiring_digital.o
avr-gcc $FLAGS.Split(' ') -c arduino_core/wiring_analog.c -o build/wiring_analog.o
avr-gcc $FLAGS.Split(' ') -c arduino_core/hooks.c -o build/hooks.o

# 2. Compilar C++ de Arduino Core & Servo
avr-g++ $FLAGS.Split(' ') -c arduino_core/HardwareSerial.cpp -o build/HardwareSerial.o
avr-g++ $FLAGS.Split(' ') -c arduino_core/HardwareSerial0.cpp -o build/HardwareSerial0.o
avr-g++ $FLAGS.Split(' ') -c arduino_core/Print.cpp -o build/Print.o
avr-g++ $FLAGS.Split(' ') -c arduino_core/Stream.cpp -o build/Stream.o
avr-g++ $FLAGS.Split(' ') -c arduino_core/WString.cpp -o build/WString.o
avr-g++ $FLAGS.Split(' ') -c arduino_core/WMath.cpp -o build/WMath.o
avr-g++ $FLAGS.Split(' ') -c arduino_core/main.cpp -o build/core_main.o
avr-g++ $FLAGS.Split(' ') -c arduino_core/avr/Servo.cpp -o build/Servo.o

# 3. Compilar C++ del Proyecto CNC
avr-g++ $FLAGS.Split(' ') -c StepperMotor.cpp -o build/StepperMotor.o
avr-g++ $FLAGS.Split(' ') -c MotionPlanner.cpp -o build/MotionPlanner.o
avr-g++ $FLAGS.Split(' ') -c GCodeParser.cpp -o build/GCodeParser.o
avr-g++ $FLAGS.Split(' ') -c Config.cpp -o build/Config.o
avr-g++ $FLAGS.Split(' ') -x c++ -c sketch.ino -o build/sketch.o

# 4. Enlazar binario .elf
Write-Host "Enlazando binario .elf ejecutable..." -ForegroundColor Cyan

avr-g++ -mmcu=atmega2560 -Os build/wiring.o build/wiring_digital.o build/wiring_analog.o build/hooks.o build/HardwareSerial.o build/HardwareSerial0.o build/Print.o build/Stream.o build/WString.o build/WMath.o build/core_main.o build/Servo.o build/StepperMotor.o build/MotionPlanner.o build/GCodeParser.o build/Config.o build/sketch.o -o build/sketch.ino.elf -lm

Copy-Item build/sketch.ino.elf build/sketch.elf -Force
Copy-Item build/sketch.ino.elf build/firmware.elf -Force

Write-Host "==========================================" -ForegroundColor Green
Write-Host " REAL ARDUINO MEGA 2560 ELF BUILT OK      " -ForegroundColor Green
Write-Host " Binario generado: build/sketch.ino.elf   " -ForegroundColor Green
Write-Host "==========================================" -ForegroundColor Green
