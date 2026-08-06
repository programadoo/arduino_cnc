import sys
import os
import time
import glob

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    print("❌ Error: La librería 'pyserial' no está instalada.")
    print("Instálala ejecutando en la terminal:  pip install pyserial")
    sys.exit(1)

def listar_puertos():
    """Retorna una lista de puertos COM disponibles."""
    puertos = serial.tools.list_ports.comports()
    return [p.device for p in puertos]

def enviar_gcode(puerto_com, baudrate, ruta_archivo):
    """Envía un archivo G-Code línea por línea esperando respuesta 'ok' del Arduino."""
    if not os.path.exists(ruta_archivo):
        print(f"❌ Error: El archivo '{ruta_archivo}' no existe.")
        return

    print(f"\n🔌 Conectando a {puerto_com} a {baudrate} baudios...")
    try:
        ser = serial.Serial(puerto_com, baudrate, timeout=3)
        time.sleep(2)  # Esperar reinicio automático del Arduino Mega
    except Exception as e:
        print(f"❌ Error al abrir el puerto {puerto_com}: {e}")
        return

    # Limpiar buffer de entrada inicial
    ser.reset_input_buffer()
    print("✅ Conexión establecida. Leyendo mensajes de bienvenida de Arduino...\n")

    # Leer mensajes de inicio del Arduino durante 2 segundos
    tiempo_inicio = time.time()
    while time.time() - tiempo_inicio < 2.0:
        if ser.in_waiting:
            linea_inicio = ser.readline().decode('utf-8', errors='ignore').strip()
            if linea_inicio:
                print(f"   [ARDUINO]: {linea_inicio}")

    with open(ruta_archivo, 'r') as f:
        lineas = f.readlines()

    lineas_limpias = []
    for l in lineas:
        l_str = l.strip()
        # Ignorar comentarios y líneas vacías
        if l_str and not l_str.startswith(';'):
            # Quitar comentarios al final de la línea
            if ';' in l_str:
                l_str = l_str.split(';')[0].strip()
            if l_str:
                lineas_limpias.append(l_str)

    total_lineas = len(lineas_limpias)
    print(f"\n🚀 Iniciando envío de {total_lineas} comandos G-Code a la CNC...\n")

    exitosos = 0
    tiempo_trazado_inicio = time.time()

    for idx, cmd in enumerate(lineas_limpias, 1):
        porcentaje = (idx / total_lineas) * 100
        print(f"[{idx:03d}/{total_lineas:03d}] ({porcentaje:5.1f}%) Enviando: {cmd}")
        
        # Enviar comando terminado en \n
        ser.write((cmd + '\n').encode('utf-8'))
        ser.flush()

        # Esperar respuesta 'ok' del Arduino
        respuesta_recibida = False
        t_espera = time.time()
        while time.time() - t_espera < 30.0:  # Timeout de 30s por movimiento
            if ser.in_waiting:
                linea_resp = ser.readline().decode('utf-8', errors='ignore').strip()
                if linea_resp:
                    print(f"       └── [ARDUINO]: {linea_resp}")
                    if 'ok' in linea_resp.lower() or 'completado' in linea_resp.lower():
                        respuesta_recibida = True
                        exitosos += 1
                        break
                    elif 'error' in linea_resp.lower():
                        print(f"⚠️ Alerta recibida del Arduino: {linea_resp}")
                        break

        if not respuesta_recibida:
            print("⚠️ Advertencia: Tiempo de espera agotado para el comando (timeout). Continuando...")

    duracion = time.time() - tiempo_trazado_inicio
    print("\n" + "=" * 55)
    print(f"🎉 TRAZADO COMPLETADO EN {duracion:.2f} SEGUNDOS")
    print(f"   Comandos enviados con éxito: {exitosos}/{total_lineas}")
    print("=" * 55)

    ser.close()

def main():
    print("=======================================================")
    print("    SENDER DE ARCHIVOS G-CODE PARA CNC PLOTTER PCB")
    print("=======================================================\n")

    puertos = listar_puertos()
    if not puertos:
        print("⚠️ No se detectaron puertos serie activos (COM).")
        print("Asegúrate de que el Arduino Mega esté conectado por USB.")
        puerto_elegido = input("\nIntroduce el nombre del puerto manualmente (ej. COM3): ").strip()
    else:
        print("Puertos COM detectados:")
        for idx, p in enumerate(puertos, 1):
            print(f"  [{idx}] {p}")
        eleccion = input(f"\nSelecciona puerto [1-{len(puertos)}] (por defecto 1): ").strip()
        if eleccion.isdigit() and 1 <= int(eleccion) <= len(puertos):
            puerto_elegido = puertos[int(eleccion) - 1]
        else:
            puerto_elegido = puertos[0]

    # Archivo por defecto o elegir
    archivo_defecto = "ejemplo_pcb.gcode"
    ruta_archivo = input(f"\nRuta del archivo G-Code (Enter para '{archivo_defecto}'): ").strip()
    if not ruta_archivo:
        ruta_archivo = archivo_defecto

    enviar_gcode(puerto_elegido, 115200, ruta_archivo)

if __name__ == '__main__':
    main()
