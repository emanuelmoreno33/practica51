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

  Serial.begin(9600);
  //se verifica que el sensor RGB este funcionando
    if (!tcs.begin())
  {
    Serial.println("Error al iniciar TCS34725");
    while (1) delay(1000);
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

Serial.print("Distance: ");
Serial.println(distance);

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

}
#define commonAnode true
byte gammatable[256];
//metodo para escribir el color
void color(int rojo, int verde, int azul)
{ 
      for (int i=0; i<256; i++) {
    float x = i;
    x /= 255;
    x = pow(x, 2.5);
    x *= 255;

    if (commonAnode) {
      gammatable[i] = 255 - x;
    } else {
      gammatable[i] = x;
    }
    //Serial.println(gammatable[i]);
  }
  
  ledcAnalogWrite(LEDC_CHANNEL_0_R, gammatable[(int)rojo]);
  ledcAnalogWrite(LEDC_CHANNEL_1_G, gammatable[(int)verde]);
  ledcAnalogWrite(LEDC_CHANNEL_2_B, gammatable[(int)azul]);
}
void printColorName(double hue,double value,double saturation)
{   
  if (hue < 15)
  {
    Serial.println("Rojo");
  }
  else if (hue < 45)
  {
    Serial.println("Naranja");
  }
  else if (hue < 90)
  {
    Serial.println("Amarillo");
  }
  else if (hue < 150)
  {
    Serial.println("Verde");
  }
  else if (hue < 210)
  {
    Serial.println("Cyan");
  }
  else if (hue < 270)
  {
    Serial.println("Azul");
  }
  else if (hue < 330)
  {
    Serial.println("Magenta");
  }
  else
  {
    Serial.println("Rojo");
  }
}
