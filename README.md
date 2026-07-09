# Asistente de IA con ESP32-S3

escribes preguntas con un joystick sobre un teclado dibujado en pantalla OLED, se mandan por WiFi a Groq (IA gratis), y la respuesta aparece en la misma pantalla — sin necesitar celular ni PC. También trae un mini-juego de esquivar cajas en 3 carriles como extra, y configura su propio WiFi solo (con red/contraseña predeterminadas, un portal web de respaldo, o comandos por terminal para gestionarlo.

# funciones y simbolo de navegacion
## ⌨️ Símbolos del teclado en pantalla

| Símbolo | Función |
|---------|---------|
| `^`     | Alterna mayúsculas/minúsculas (solo en la página de letras) |
| `&`     | Cambia entre la página de letras y la de números/símbolos |
| `_`     | Espacio |
| `<`     | Borra el último carácter |
| `#`     | Envía la pregunta a la IA |
| `@`     | Abre el mini-juego de esquivar cajas |

## 🕹️ Controles del joystick

**Modo escritura:**
- Mueve el joystick para navegar por las letras/símbolos del teclado
- Presiona el botón (SW) para seleccionar la letra resaltada

**Modo respuesta:**
- Arriba/abajo para pasar de página si la respuesta es larga
- Presiona el botón (SW) para volver a escribir una nueva pregunta

**Modo juego:**
- Izquierda/derecha para cambiar de carril
- Presiona el botón (SW) para salir del juego en cualquier momento

## 📶 Configuración de WiFi

El dispositivo intenta conectarse en este orden:

1. **WiFi guardado** — si ya conectaste exitosamente antes, se conecta directo sin preguntar nada.
2. **Redes predeterminadas** — prueba automáticamente las redes definidas en el código (`redesGuardadas[]`).
3. **Portal de configuración** — si ninguna de las anteriores funciona, el ESP32 crea su propia red WiFi para que configures una manualmente.

### Cómo entrar al portal de configuración

Cuando el dispositivo no logra conectarse con ninguna red guardada ni predeterminada, automáticamente:

1. Crea una red WiFi llamada **`ESP32-Config`** (verás el mensaje en la pantalla OLED junto con la IP).
2. Desde tu celular o computadora, ve a **Ajustes → WiFi** y conéctate a esa red:
   - **Red:** `ESP32-Config`
   - **Contraseña:** `12345678`
3. Una vez conectado, abre cualquier navegador y entra a la ip que muestra el esp32-s3:
4. Va a aparecer un formulario simple con dos campos: **nombre de red (SSID)** y **contraseña**.
5. Llena tus datos reales de WiFi y presiona **"Guardar y Conectar"**.
6. El ESP32 se reinicia solo y se conecta a la red que acabas de configurar.

> **Nota:** mientras estás conectado a `ESP32-Config`, tu celular/PC no va a tener acceso a internet — es una red aislada solo para la configuración. Es normal.

### Comandos disponibles por Monitor Serie

Con el ESP32 conectado por USB y el Monitor Serie abierto (115200 baudios), puedes escribir:

| Comando | Función |
|---------|---------|
| `borrar wifi` | Borra el WiFi guardado en memoria y reinicia el dispositivo |
| `ver wifi` | Muestra en el Monitor Serie y en pantalla la red y contraseña guardadas actualmente |

## Hardware necesario
- ESP32-S3
- Pantalla OLED 0.96" SPI de 6 pines o de 4 pines (SSD1306)
- Joystick analógico tipo PS2

## Conexiones 1 (su codigo es el "codigo 1")

###  OLED de 6 pines (SPI)
| Pin de la OLED | Conecta a (ESP32-S3) |
|-----------------|------------------------|
| GND             | GND                    |
| VCC             | 3.3V                   |
| SCK             | GPIO 12                |
| SDA             | GPIO 11                |
| RES             | GPIO 10                |
| DC              | GPIO 13                |

### Joystick (KY-023)

| Pin del Joystick | Conecta a (ESP32-S3) |
|-------------------|------------------------|
| GND               | GND                    |
| VCC (+5V)         | 3.3V                   |
| VRx               | GPIO 1                 |
| VRy               | GPIO 2                 |
| SW                | GPIO 14                |


## Conexiones 2 (su codigo es el "codigo 2")
(Puede estar sujeta a errores/bug por que no tengo
una pantalla de 4 pines para probar el codigo)
### 📺 OLED de 4 pines (I2C)

| Pin de la OLED | Conecta a (ESP32-S3) |
|-----------------|------------------------|
| GND             | GND                    |
| VCC             | 3.3V                   |
| SCL             | GPIO 9                 |
| SDA             | GPIO 8                 |

### Joystick (KY-023)

| Pin del Joystick | Conecta a (ESP32-S3) |
|-------------------|------------------------|
| GND               | GND                    |
| VCC (+5V)         | 3.3V                   |
| VRx               | GPIO 1                 |
| VRy               | GPIO 2                 |
| SW                | GPIO 14                |


## Instalación
1. Instala las librerías: ArduinoJson, Adafruit GFX, Adafruit SSD1306
2. Consigue tu API key gratis en console.groq.com
3. Pega tu API key en el código...

## Uso
- Mueve el joystick para navegar el teclado
- Presiona el botón interno del joystick para seleccionar una letra o accion
- Selecciona '#' para enviar tu pregunta
...
