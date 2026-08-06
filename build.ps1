$env:PATH = "$PSScriptRoot\tools\avr-gcc-14.1.0-x64-windows\bin;$PSScriptRoot\tools\avr-gcc-14.1.0-x64-windows\libexec\gcc\avr\14.1.0;" + $env:PATH
if (-not (Test-Path "$PSScriptRoot\build")) { New-Item -ItemType Directory -Path "$PSScriptRoot\build" }

Write-Host "Compilando firmware para ATmega2560..." -ForegroundColor Cyan

avr-g++ -mmcu=atmega2560 -DF_CPU=16000000UL -Os -I. -Iavr_inc -c StepperMotor.cpp -o build/StepperMotor.o
avr-g++ -mmcu=atmega2560 -DF_CPU=16000000UL -Os -I. -Iavr_inc -c MotionPlanner.cpp -o build/MotionPlanner.o
avr-g++ -mmcu=atmega2560 -DF_CPU=16000000UL -Os -I. -Iavr_inc -c GCodeParser.cpp -o build/GCodeParser.o
avr-g++ -mmcu=atmega2560 -DF_CPU=16000000UL -Os -I. -Iavr_inc -c Config.cpp -o build/Config.o
avr-g++ -mmcu=atmega2560 -DF_CPU=16000000UL -Os -I. -Iavr_inc -x c++ -c sketch.ino -o build/sketch.o
avr-g++ -mmcu=atmega2560 -DF_CPU=16000000UL -Os -I. -Iavr_inc -x c++ -c avr_inc/main.cpp -o build/main.o

avr-g++ -mmcu=atmega2560 build/StepperMotor.o build/MotionPlanner.o build/GCodeParser.o build/Config.o build/sketch.o build/main.o -o build/sketch.ino.elf

Copy-Item build/sketch.ino.elf build/sketch.elf -Force
Copy-Item build/sketch.ino.elf build/firmware.elf -Force

Write-Host "==========================================" -ForegroundColor Green
Write-Host " COMPILACION EXITOSA PARA ARDUINO MEGA    " -ForegroundColor Green
Write-Host " Binario generado: build/sketch.ino.elf   " -ForegroundColor Green
Write-Host "==========================================" -ForegroundColor Green
