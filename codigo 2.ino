#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WebServer.h>
#include <Preferences.h>

// ---- TU API KEY ----
const char* groqApiKey = "INGRESA_API_DE_GROP_AQUI";
// --------------------

// ---- LISTA DE WIFIS PREDETERMINADOS (opcional) ----
struct RedWiFi {
  const char* ssid;
  const char* password;
};
// ---- LISTA DE WIFIS DE EJEMPLO
RedWiFi redesGuardadas[] = {
  {"carniceria", "chapala"},
  {"OtraRedCasa", "otraClave123"},
  {"WiFiDelTaller", "claveTaller456"}
};

const int TOTAL_REDES = sizeof(redesGuardadas) / sizeof(redesGuardadas[0]);
// -----------------------------------------

const char* groqUrl = "https://api.groq.com/openai/v1/chat/completions";

// ---- Pines OLED (I2C) ----
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET   -1
#define OLED_ADDR    0x3C

#define PIN_SDA 8
#define PIN_SCL 9

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---- Pines Joystick ----
#define PIN_VRX 1
#define PIN_VRY 2
#define PIN_SW  14

Preferences prefs;
WebServer server(80);
String ssidGuardado, passGuardado;

// ---- Teclado en pantalla: 2 paginas ----
// '@' = abrir el juego de esquivar cajas
const int FILAS = 4;
const int COLS = 10;

const char teclado_letras[FILAS][COLS + 1] = {
  "ABCDEFGHIJ",
  "KLMNOPQRST",
  "UVWXYZ    ",
  "^&_<#@    "
};

const char teclado_simbolos[FILAS][COLS + 1] = {
  "1234567890",
  ".,!?-+*/=@",
  "()[]{}:;%$",
  "^&_<#@    "
};

enum Pagina { LETRAS, SIMBOLOS };
Pagina paginaActual = LETRAS;
bool mayusculas = true;

int cursorFila = 0;
int cursorCol = 0;
bool ejeLibre = true;
bool botonPresionadoAntes = false;

String buffer = "";
String respuestaActual = "";

enum Modo { ESCRIBIENDO, RESPUESTA, JUEGO };
Modo modoActual = ESCRIBIENDO;
bool respuestaYaMostrada = false;

// ---- Paginado de la respuesta ----
#define MAX_LINEAS 60
#define CHARS_POR_LINEA 20
#define LINEAS_POR_PAGINA 7

String lineasRespuesta[MAX_LINEAS];
int lineasCount = 0;
int paginaRespuestaActual = 0;
int paginaAnteriorMostrada = -1;
bool ejeLibreRespuesta = true;

// ---- Juego: esquivar cajas en carretera de 3 carriles ----
#define NUM_CARRILES 3
#define MAX_CAJAS 3

struct Caja {
  int carril;
  int y;
  bool activa;
};
Caja cajas[MAX_CAJAS];

int carrilActual = 1;
bool ejeLibreJuego = true;

const int laneWidth = SCREEN_WIDTH / NUM_CARRILES;
const int carAncho = 14;
const int carAlto = 12;
const int carritoY = 47;
const int cajaLado = 9;

int puntuacion = 0;
bool juegoTerminado = false;
unsigned long ultimoSpawn = 0;
int velocidadCaida = 2;

// ---------- OLED texto simple ----------
void mostrarEnOLED(String texto) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(texto);
  display.display();
}

char aplicarCaso(char c) {
  if (paginaActual == LETRAS && c >= 'A' && c <= 'Z') {
    return mayusculas ? c : (char)tolower(c);
  }
  return c;
}

// ---------- Quitar acentos/UTF-8 que la OLED no puede mostrar ----------
String limpiarAcentos(String texto) {
  String resultado = "";
  for (int i = 0; i < texto.length(); i++) {
    uint8_t c = texto[i];

    if (c == 0xC3 && i + 1 < texto.length()) {
      uint8_t siguiente = texto[i + 1];
      i++;

      switch (siguiente) {
        case 0xA1: resultado += 'a'; break; // á
        case 0xA9: resultado += 'e'; break; // é
        case 0xAD: resultado += 'i'; break; // í
        case 0xB3: resultado += 'o'; break; // ó
        case 0xBA: resultado += 'u'; break; // ú
        case 0xB1: resultado += 'n'; break; // ñ
        case 0x81: resultado += 'A'; break; // Á
        case 0x89: resultado += 'E'; break; // É
        case 0x8D: resultado += 'I'; break; // Í
        case 0x93: resultado += 'O'; break; // Ó
        case 0x9A: resultado += 'U'; break; // Ú
        case 0x91: resultado += 'N'; break; // Ñ
        default: break;
      }
    } else if (c < 128) {
      resultado += (char)c;
    }
  }
  return resultado;
}

// ---------- Pantalla de escritura (buffer + teclado) ----------
void mostrarPantallaEscritura() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  String visible = buffer;
  if (visible.length() > 40) {
    visible = visible.substring(visible.length() - 40);
  }
  String linea1 = "";
  String linea2 = "";
  if (visible.length() <= 20) {
    linea1 = visible;
  } else {
    linea1 = visible.substring(0, visible.length() - 20);
    linea2 = visible.substring(visible.length() - 20);
  }
  display.setCursor(0, 0);
  display.println(linea1);
  display.println(linea2);

  display.setCursor(122, 0);
  if (paginaActual == SIMBOLOS) {
    display.print("1");
  } else {
    display.print(mayusculas ? "A" : "a");
  }

  display.drawLine(0, 18, 128, 18, SSD1306_WHITE);

  int startY = 20;
  int cellW = 12;
  int cellH = 10;

  for (int fila = 0; fila < FILAS; fila++) {
    for (int col = 0; col < COLS; col++) {
      int x = col * cellW;
      int y = startY + fila * cellH;

      char cOriginal = (paginaActual == LETRAS)
                          ? teclado_letras[fila][col]
                          : teclado_simbolos[fila][col];
      char cMostrar = aplicarCaso(cOriginal);

      bool esCursor = (fila == cursorFila && col == cursorCol);

      if (esCursor) {
        display.fillRect(x, y, cellW, cellH, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
      } else {
        display.setTextColor(SSD1306_WHITE);
      }

      if (cOriginal != ' ') {
        display.setCursor(x + 3, y + 1);
        display.print(cMostrar);
      }
    }
  }

  display.display();
}

// ---------- Preparar y mostrar respuesta paginada ----------
void prepararLineasRespuesta(String texto) {
  lineasCount = 0;
  int i = 0;
  while (i < (int)texto.length() && lineasCount < MAX_LINEAS) {
    int fin = min(i + CHARS_POR_LINEA, (int)texto.length());
    lineasRespuesta[lineasCount] = texto.substring(i, fin);
    lineasCount++;
    i = fin;
  }
  if (lineasCount == 0) {
    lineasRespuesta[0] = "";
    lineasCount = 1;
  }
}

int totalPaginasRespuesta() {
  int total = (lineasCount + LINEAS_POR_PAGINA - 1) / LINEAS_POR_PAGINA;
  return (total < 1) ? 1 : total;
}

void mostrarPaginaRespuesta() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  int total = totalPaginasRespuesta();
  int inicio = paginaRespuestaActual * LINEAS_POR_PAGINA;

  for (int i = 0; i < LINEAS_POR_PAGINA; i++) {
    int idx = inicio + i;
    if (idx < lineasCount) {
      display.setCursor(0, i * 8);
      display.println(lineasRespuesta[idx]);
    }
  }

  display.setCursor(0, 56);
  if (total > 1) {
    display.print("Pag " + String(paginaRespuestaActual + 1) + "/" + String(total) + " (abajo=+)");
  } else {
    display.print("SW=volver a escribir");
  }

  display.display();
}

void leerJoystickRespuesta() {
  int y = analogRead(PIN_VRY);
  int total = totalPaginasRespuesta();

  if (ejeLibreRespuesta) {
    if (y > 3000) {
      if (paginaRespuestaActual < total - 1) paginaRespuestaActual++;
      ejeLibreRespuesta = false;
    } else if (y < 1000) {
      if (paginaRespuestaActual > 0) paginaRespuestaActual--;
      ejeLibreRespuesta = false;
    }
  }

  if (y > 1500 && y < 2500) {
    ejeLibreRespuesta = true;
  }
}

// ---------- Joystick (pantalla de escritura) ----------
void leerJoystick() {
  int x = analogRead(PIN_VRX);
  int y = analogRead(PIN_VRY);

  if (ejeLibre) {
    if (x > 3000) { cursorCol++; ejeLibre = false; }
    else if (x < 1000) { cursorCol--; ejeLibre = false; }
    else if (y > 3000) { cursorFila++; ejeLibre = false; }
    else if (y < 1000) { cursorFila--; ejeLibre = false; }
  }

  if (x > 1500 && x < 2500 && y > 1500 && y < 2500) {
    ejeLibre = true;
  }

  if (cursorCol < 0) cursorCol = COLS - 1;
  if (cursorCol >= COLS) cursorCol = 0;
  if (cursorFila < 0) cursorFila = FILAS - 1;
  if (cursorFila >= FILAS) cursorFila = 0;
}

bool leerBotonPresionado() {
  bool estado = (digitalRead(PIN_SW) == LOW);
  bool presionado = false;
  if (estado && !botonPresionadoAntes) {
    presionado = true;
  }
  botonPresionadoAntes = estado;
  return presionado;
}

// ---------- Juego: esquivar cajas en carretera ----------
int xCentroCarril(int carril) {
  return carril * laneWidth + laneWidth / 2;
}

void iniciarJuego() {
  for (int i = 0; i < MAX_CAJAS; i++) cajas[i].activa = false;
  carrilActual = 1;
  puntuacion = 0;
  juegoTerminado = false;
  velocidadCaida = 2;
  ultimoSpawn = millis();
  randomSeed(millis());
}

void leerJoystickJuego() {
  int x = analogRead(PIN_VRX);

  if (ejeLibreJuego) {
    if (x > 3000 && carrilActual < NUM_CARRILES - 1) {
      carrilActual++;
      ejeLibreJuego = false;
    } else if (x < 1000 && carrilActual > 0) {
      carrilActual--;
      ejeLibreJuego = false;
    }
  }

  if (x > 1500 && x < 2500) {
    ejeLibreJuego = true;
  }
}

void actualizarJuego() {
  leerJoystickJuego();

  if (millis() - ultimoSpawn > 900) {
    for (int i = 0; i < MAX_CAJAS; i++) {
      if (!cajas[i].activa) {
        cajas[i].activa = true;
        cajas[i].carril = random(0, NUM_CARRILES);
        cajas[i].y = 12;
        break;
      }
    }
    ultimoSpawn = millis();
  }

  for (int i = 0; i < MAX_CAJAS; i++) {
    if (cajas[i].activa) {
      cajas[i].y += velocidadCaida;

      if (cajas[i].y > SCREEN_HEIGHT) {
        cajas[i].activa = false;
        puntuacion++;
        if (puntuacion % 5 == 0 && velocidadCaida < 8) velocidadCaida++;
      } else if (cajas[i].carril == carrilActual) {
        bool colisionY = (cajas[i].y + cajaLado >= carritoY) && (cajas[i].y <= carritoY + carAlto);
        if (colisionY) juegoTerminado = true;
      }
    }
  }
}

void dibujarCarro(int x, int y) {
  // carroceria
  display.fillRect(x + 1, y + 5, carAncho - 2, 6, SSD1306_WHITE);
  // cabina/techo
  display.fillRect(x + 3, y, carAncho - 6, 6, SSD1306_WHITE);
  // parabrisas (recorte oscuro)
  display.fillRect(x + 4, y + 1, carAncho - 8, 3, SSD1306_BLACK);
  // llantas
  display.fillRect(x, y + 10, 3, 2, SSD1306_WHITE);
  display.fillRect(x + carAncho - 3, y + 10, 3, 2, SSD1306_WHITE);
}

void dibujarCarretera() {
  int offset = (millis() / 80) % 8;
  for (int carril = 1; carril < NUM_CARRILES; carril++) {
    int x = carril * laneWidth;
    for (int y = 11 - offset; y < SCREEN_HEIGHT; y += 8) {
      if (y > 10) display.drawFastVLine(x, y, 4, SSD1306_WHITE);
    }
  }
}

void mostrarJuego() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.print("Pts:" + String(puntuacion));

  display.setCursor(120, 0);
  display.print("X");

  display.drawLine(0, 10, SCREEN_WIDTH, 10, SSD1306_WHITE);

  if (juegoTerminado) {
    display.setCursor(20, 24);
    display.print("PERDISTE");
    display.setCursor(10, 36);
    display.print("Puntos: " + String(puntuacion));
    display.setCursor(0, 54);
    display.print("SW = salir");
  } else {
    dibujarCarretera();

    for (int i = 0; i < MAX_CAJAS; i++) {
      if (cajas[i].activa) {
        int cx = xCentroCarril(cajas[i].carril) - cajaLado / 2;
        display.fillRect(cx, cajas[i].y, cajaLado, cajaLado, SSD1306_WHITE);
      }
    }

    int carX = xCentroCarril(carrilActual) - carAncho / 2;
    dibujarCarro(carX, carritoY);
  }

  display.display();
}

// ---------- Comandos por Monitor Serie ----------
void revisarComandoSerial() {
  if (Serial.available()) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();
    comando.toLowerCase();

    if (comando == "borrar wifi") {
      Serial.println("Borrando todas las redes WiFi guardadas...");
      mostrarEnOLED("Borrando WiFi\nguardado...");

      prefs.begin("wifi", false);
      prefs.clear();
      prefs.end();

      delay(1500);
      Serial.println("Listo. Reiniciando...");
      ESP.restart();
    }
    else if (comando == "ver wifi") {
      prefs.begin("wifi", false);
      String ssidActual = prefs.getString("ssid", "");
      String passActual = prefs.getString("pass", "");
      prefs.end();

      Serial.println("=== WiFi guardado actualmente ===");
      if (ssidActual == "") {
        Serial.println("No hay ninguna red guardada.");
        mostrarEnOLED("No hay WiFi\nguardado");
      } else {
        Serial.print("Red: ");
        Serial.println(ssidActual);
        Serial.print("Clave: ");
        Serial.println(passActual);
        mostrarEnOLED("Red: " + ssidActual + "\nClave: " + passActual);
      }
      Serial.println("==================================");

      delay(3000);
    }
  }
}

// ---------- Portal de configuracion WiFi ----------
const char* paginaConfig = R"rawliteral(
<!DOCTYPE html>
<html>
<head><title>Configurar WiFi</title></head>
<body style="font-family: sans-serif; text-align: center; padding-top: 50px;">
  <h2>Configura tu WiFi</h2>
  <form action="/guardar" method="POST">
    Nombre de red (SSID):<br>
    <input type="text" name="ssid"><br><br>
    Contraseña:<br>
    <input type="password" name="pass"><br><br>
    <input type="submit" value="Guardar y Conectar">
  </form>
</body>
</html>
)rawliteral";

void manejarRaiz() {
  server.send(200, "text/html", paginaConfig);
}

void manejarGuardar() {
  String nuevoSSID = server.arg("ssid");
  String nuevoPass = server.arg("pass");

  prefs.begin("wifi", false);
  prefs.putString("ssid", nuevoSSID);
  prefs.putString("pass", nuevoPass);
  prefs.end();

  server.send(200, "text/html", "<h2>Guardado! Reiniciando...</h2>");
  delay(2000);
  ESP.restart();
}

void iniciarPortalConfiguracion() {
  Serial.println("=== Creando red de configuracion ===");

  WiFi.softAP("ESP32-Config", "12345678");
  IPAddress ip = WiFi.softAPIP();

  mostrarEnOLED("Red: ESP32-Config\nClave: 12345678\nIP: " + ip.toString());

  Serial.print("Entra a esta IP desde el navegador: ");
  Serial.println(ip);
  Serial.println("(Tambien puedes escribir 'borrar wifi' aqui para reiniciar)");

  server.on("/", manejarRaiz);
  server.on("/guardar", HTTP_POST, manejarGuardar);
  server.begin();

  while (true) {
    server.handleClient();
    revisarComandoSerial();
    delay(10);
  }
}

// ---------- Conexion WiFi (varias redes predeterminadas) ----------
void configurarWiFi() {
  prefs.begin("wifi", false);
  ssidGuardado = prefs.getString("ssid", "");
  passGuardado = prefs.getString("pass", "");
  prefs.end();

  if (ssidGuardado == "" && TOTAL_REDES > 0) {
    for (int i = 0; i < TOTAL_REDES; i++) {
      Serial.print("Probando red: ");
      Serial.println(redesGuardadas[i].ssid);
      mostrarEnOLED("Probando:\n" + String(redesGuardadas[i].ssid));

      WiFi.begin(redesGuardadas[i].ssid, redesGuardadas[i].password);

      int intentos = 0;
      while (WiFi.status() != WL_CONNECTED && intentos < 15) {
        delay(500);
        Serial.print(".");
        intentos++;
      }

      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n¡Conectado!");
        ssidGuardado = redesGuardadas[i].ssid;
        passGuardado = redesGuardadas[i].password;

        prefs.begin("wifi", false);
        prefs.putString("ssid", ssidGuardado);
        prefs.putString("pass", passGuardado);
        prefs.end();

        break;
      } else {
        Serial.println("\nNo conecto, probando siguiente...");
        WiFi.disconnect();
      }
    }
  }

  if (ssidGuardado == "" && WiFi.status() != WL_CONNECTED) {
    iniciarPortalConfiguracion();
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Conectando a: ");
    Serial.println(ssidGuardado);
    mostrarEnOLED("Conectando a:\n" + ssidGuardado);

    WiFi.begin(ssidGuardado.c_str(), passGuardado.c_str());

    int intentos = 0;
    while (WiFi.status() != WL_CONNECTED && intentos < 20) {
      delay(500);
      Serial.print(".");
      intentos++;
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n¡WiFi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nNo se pudo conectar a ninguna red. Borrando datos...");
    prefs.begin("wifi", false);
    prefs.clear();
    prefs.end();
    ESP.restart();
  }
}

// ---------- Groq ----------
String preguntarAGroq(String pregunta) {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, groqUrl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", String("Bearer ") + groqApiKey);

  StaticJsonDocument<512> reqDoc;
  reqDoc["model"] = "llama-3.1-8b-instant";
  JsonArray messages = reqDoc.createNestedArray("messages");

  JsonObject msgSistema = messages.createNestedObject();
  msgSistema["role"] = "system";
  msgSistema["content"] = "Responde siempre en español, de forma breve y concisa, sin usar acentos ni la letra ñ (usa n en su lugar).";

  JsonObject msg = messages.createNestedObject();
  msg["role"] = "user";
  msg["content"] = pregunta;

  String reqBody;
  serializeJson(reqDoc, reqBody);

  int httpCode = http.POST(reqBody);
  String respuesta = "";

  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument resDoc(4096);
    deserializeJson(resDoc, payload);
    respuesta = resDoc["choices"][0]["message"]["content"].as<String>();
  } else {
    respuesta = "Error HTTP: " + String(httpCode);
  }

  http.end();

  respuesta = limpiarAcentos(respuesta);
  return respuesta;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(PIN_SW, INPUT_PULLUP);

  Wire.begin(PIN_SDA, PIN_SCL);

  Serial.println("=== Iniciando OLED ===");
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("Error: no se pudo iniciar la OLED.");
  } else {
    display.setTextWrap(false);
    mostrarEnOLED("Iniciando...");
  }

  configurarWiFi();

  modoActual = ESCRIBIENDO;
  Serial.println("\n=== Listo. Usa el joystick para escribir ===");
  Serial.println("=== Comandos disponibles en esta terminal ===");
  Serial.println("  borrar wifi  -> borra el WiFi guardado y reinicia");
  Serial.println("  ver wifi     -> muestra la red y clave guardada actualmente");
  Serial.println("==============================================");
}

void loop() {
  revisarComandoSerial();

  if (modoActual == ESCRIBIENDO) {
    leerJoystick();
    mostrarPantallaEscritura();

    if (leerBotonPresionado()) {
      char actual = (paginaActual == LETRAS)
                       ? teclado_letras[cursorFila][cursorCol]
                       : teclado_simbolos[cursorFila][cursorCol];

      if (actual == ' ') {
        // celda vacia, no hace nada
      } else if (actual == '#') {
        if (buffer.length() > 0) {
          mostrarEnOLED("Pensando...");
          respuestaActual = preguntarAGroq(buffer);
          modoActual = RESPUESTA;
          respuestaYaMostrada = false;
          paginaRespuestaActual = 0;
          paginaAnteriorMostrada = -1;
        }
      } else if (actual == '<') {
        if (buffer.length() > 0) buffer.remove(buffer.length() - 1);
      } else if (actual == '^') {
        mayusculas = !mayusculas;
      } else if (actual == '&') {
        paginaActual = (paginaActual == LETRAS) ? SIMBOLOS : LETRAS;
      } else if (actual == '@') {
        iniciarJuego();
        modoActual = JUEGO;
      } else if (actual == '_') {
        buffer += ' ';
      } else {
        buffer += aplicarCaso(actual);
      }
    }

    delay(80);
  }
  else if (modoActual == RESPUESTA) {
    if (!respuestaYaMostrada) {
      prepararLineasRespuesta(respuestaActual);
      paginaRespuestaActual = 0;
      mostrarPaginaRespuesta();
      paginaAnteriorMostrada = paginaRespuestaActual;
      respuestaYaMostrada = true;
    }

    leerJoystickRespuesta();

    if (paginaRespuestaActual != paginaAnteriorMostrada) {
      mostrarPaginaRespuesta();
      paginaAnteriorMostrada = paginaRespuestaActual;
    }

    if (leerBotonPresionado()) {
      buffer = "";
      respuestaActual = "";
      modoActual = ESCRIBIENDO;
    }

    delay(80);
  }
  else if (modoActual == JUEGO) {
    if (!juegoTerminado) {
      actualizarJuego();
    }
    mostrarJuego();

    if (leerBotonPresionado()) {
      modoActual = ESCRIBIENDO;
    }

    delay(50);
  }
}
// hecho por heo_717
