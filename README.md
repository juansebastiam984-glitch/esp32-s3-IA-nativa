# Asistente de IA con ESP32-S3

Un dispositivo standalone que usa un joystick para escribir preguntas
en un teclado en pantalla, las manda a la API de Groq, y muestra la
respuesta en una pantalla OLED.

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
