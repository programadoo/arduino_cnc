; ========================================================
; PROYECTO REAL PCB: CIRCUITO INTEGRADO 555 Y PISTAS PCB
; Generado para Arduino CNC Plotter (G0, G1, G90, G91, M3, M5)
; ========================================================

; 1. Configuración Inicial
G90          ; Coordenadas absolutas
M5           ; Asegurar lápiz levantado
G0 X0 Y0     ; Ir a posición de origen (0,0)

; 2. Trazado de Contorno Exterior de la Placa PCB (10cm x 8cm)
M5           ; Levantar lápiz
G0 X0.5 Y0.5 ; Esquina inferior izquierda del marco
M3           ; Bajar lápiz sobre la placa
G1 X10.5 Y0.5 ; Borde inferior
G1 X10.5 Y8.5 ; Borde derecho
G1 X0.5 Y8.5  ; Borde superior
G1 X0.5 Y0.5  ; Borde izquierdo (cerrar marco)
M5           ; Levantar lápiz

; 3. Trazado de Pistas del Integrado IC1 (Pista de Alimentación VCC)
G0 X2.0 Y2.0
M3
G1 X8.0 Y2.0 ; Pista VCC principal
M5

; 4. Trazado de Pista de Masa GND
G0 X2.0 Y7.0
M3
G1 X8.0 Y7.0 ; Pista GND principal
M5

; 5. Trazado de Pista de Señal en Zic-Zac (Ruta de Componente)
G0 X3.0 Y2.0
M3
G1 X3.0 Y4.0
G1 X5.0 Y4.0
G1 X5.0 Y6.0
G1 X7.0 Y6.0
G1 X7.0 Y7.0
M5

; 6. Pad de Conexión de Salida
G0 X8.0 Y4.5
M3
G1 X9.5 Y4.5
M5

; 7. Retorno Seguro a Origen
G0 X0 Y0     ; Ir a Home (0,0)
M5           ; Lápiz arriba

; ========================================================
; FIN DEL TRAZADO PCB
; ========================================================
