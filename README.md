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
