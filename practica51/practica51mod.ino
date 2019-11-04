//Sensor RGB
#include <ColorConverterLib.h>
#include <Wire.h>
#include "Adafruit_TCS34725.h"
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_1X);

//Sensor Ultrasonico
const int trigPin = 2;
const int echoPin = 5;
long duration;
int distance;

//Led RGB
//canales para comunicarse via PMW
#define LEDC_CHANNEL_0_R     0
#define LEDC_CHANNEL_1_G     1
#define LEDC_CHANNEL_2_B     2

//Bits de precision para el timer
#define LEDC_TIMER_13_BIT  13

//Frecuencia base para el led
#define LEDC_BASE_FREQ     5000

//pines usados para enviar el PWM
#define LED_PIN_R            18
#define LED_PIN_G            19
#define LED_PIN_B            23

//metodo para crear el AnalogWrite
void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
  // calculate duty, 8191 from 2 ^ 13 - 1
  uint32_t duty = (8191 / valueMax) * min(value, valueMax);
  ledcWrite(channel, duty);
}

//wifi
int contadorcolor[]={0,0,0,0,0,0,0};

#include <WiFi.h>
WiFiServer server(80);
const char* ssid     = "";
const char* password = "";
int contconexion = 0;
String header; // Variable para guardar el HTTP request


String pagina = "<!DOCTYPE html>"
"<html>"
"<head>"
"<meta charset='utf-8' />"
"<title>Servidor Web ESP32</title>"
"</head>"
"<body style='background-color:#090'>"
"<center>"
"<h1>Colores del sensor</h1>"
;

String piedepagina = "<p><a href='/limpiar'><button style='height:50px;width:100px;background-color: #4CAF50;display: inline-block;font-size: 16px;'>OFF</button></a></p>"
"</center>"
"</body>"
"</html>";

void setup() {
  //Se inicializan los leds indicando el canal, frecuencia y el timer, se ancla al pin que le indicamos un canal antes inicializado
  ledcSetup(LEDC_CHANNEL_0_R, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(LED_PIN_R, LEDC_CHANNEL_0_R);
  ledcSetup(LEDC_CHANNEL_1_G, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(LED_PIN_G, LEDC_CHANNEL_1_G);
  ledcSetup(LEDC_CHANNEL_2_B, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(LED_PIN_B, LEDC_CHANNEL_2_B);

  //se inicializa el sensor ultrasonico, indicando los pines donde esta
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT); 

  //wifi
  Serial.begin(115200);
  //se verifica que el sensor RGB este funcionando
    if (!tcs.begin())
  {
    Serial.println("Error al iniciar TCS34725");
    while (1) delay(1000);
  }

  WiFi.begin(ssid, password);
  //Cuenta hasta 50 si no se puede conectar lo cancela
  while (WiFi.status() != WL_CONNECTED and contconexion <50) { 
    ++contconexion;
    delay(500);
    Serial.print(".");
  }
  if (contconexion <50) {
      
      Serial.println("");
      Serial.println("WiFi conectado");
      Serial.println(WiFi.localIP());
      server.begin();
  }
  else { 
      Serial.println("");
      Serial.println("Error de conexion");
  }
}

int contador=0;


void loop() {
  
digitalWrite(trigPin, LOW);
delayMicroseconds(2);

digitalWrite(trigPin, HIGH);
delayMicroseconds(10);
digitalWrite(trigPin, LOW);

duration = pulseIn(echoPin, HIGH);

distance= duration*0.034/2;

if(distance <= 5)
{
  contador++;
  if (contador == 3)
  {
  uint16_t clear, red, green, blue;
  tcs.setInterrupt(false);
  delay(60); // Cuesta 50ms capturar el color
  tcs.getRawData(&red, &green, &blue, &clear);
  tcs.setInterrupt(true);

  // Hacer rgb medición relativa
  uint32_t sum = clear;
  float r, g, b;
  r = red; r /= sum;
  g = green; g /= sum;
  b = blue; b /= sum;

  color(r,g,b);
  
  // Escalar rgb a bytes
  r *= 256; g *= 256; b *= 256;

  // Convertir a hue, saturation, value
  double hue, saturation, value;
  
  ColorConverter::RgbToHsv(static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b), hue, saturation, value);

  // Mostrar nombre de color
  printColorName(hue * 360,saturation*360,value*360);

  delay(1000);
  contador =0;
  }
}

//inicializacion de wifi
WiFiClient client = server.available();   // Escucha a los clientes entrantes

  if (client) {                             // Si se conecta un nuevo cliente
    Serial.println("New Client.");          // 
    String currentLine = "";                //
    while (client.connected()) {            // loop mientras el cliente está conectado
      if (client.available()) {             // si hay bytes para leer desde el cliente
        char c = client.read();             // lee un byte
        Serial.write(c);                    // imprime ese byte en el monitor serial
        header += c;
        if (c == '\n') {                    // si el byte es un caracter de salto de linea
          // si la nueva linea está en blanco significa que es el fin del 
          // HTTP request del cliente, entonces respondemos:
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // enciende y apaga el GPIO
            if (header.indexOf("GET /limpiar") >= 0) {
              Serial.println("Borrando registros por orden de pagina web");
              for (byte i = 0; i < 7; i = i + 1) 
              {
                  contadorcolor[i]=0;
              }
            }
            
            // Muestra la página web
            client.println(pagina);
            client.println("Rojo:");
            client.println(contadorcolor[0]);
            client.println("<br>");
            client.println("Naranja");
            client.println(contadorcolor[1]);
            client.println("<br>");
            client.println("Amarillo");
            client.println(contadorcolor[2]);
            client.println("<br>");
            client.println("Verde");
            client.println(contadorcolor[3]);
            client.println("<br>");
            client.println("Cyan");
            client.println(contadorcolor[4]);
            client.println("<br>");
            client.println("Azul");
            client.println(contadorcolor[5]);
            client.println("<br>");
            client.println("Magenta");
            client.println(contadorcolor[6]);
            client.println("<br>");
            client.println(piedepagina);
            
            // la respuesta HTTP temina con una linea en blanco
            client.println();
            break;
          } else { // si tenemos una nueva linea limpiamos currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // si C es distinto al caracter de retorno de carro
          currentLine += c;      // lo agrega al final de currentLine
        }
      }
    }
    // Limpiamos la variable header
    header = "";
    // Cerramos la conexión
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

}
//metodo para escribir el color
void color(int rojo, int verde, int azul)
{ 
  ledcAnalogWrite(LEDC_CHANNEL_0_R,rojo);
  ledcAnalogWrite(LEDC_CHANNEL_1_G,verde);
  ledcAnalogWrite(LEDC_CHANNEL_2_B,azul);
}
void printColorName(double hue,double value,double saturation)
{   
  if (hue < 15)
  {
    Serial.println("Rojo");
    color(255,0,0);
    contadorcolor[0]++;
  }
  else if (hue < 45)
  {
    Serial.println("Naranja");
    color(255,127,0);
    contadorcolor[1]++;
  }
  else if (hue < 90)
  {
    Serial.println("Amarillo");
    color(247,255,0);
    contadorcolor[2]++;
  }
  else if (hue < 150)
  {
    Serial.println("Verde");
    color(0,255,0);
    contadorcolor[3]++;
  }
  else if (hue < 210)
  {
    Serial.println("Cyan");
    color(0,255,255);
    contadorcolor[4]++;
  }
  else if (hue < 270)
  {
    Serial.println("Azul");
    color(0,0,255);
    contadorcolor[5]++;
  }
  else if (hue < 330)
  {
    Serial.println("Magenta");
    color(173,0,255);
    contadorcolor[6]++;
  }
  else
  {
    Serial.println("Rojo");
  }
}
