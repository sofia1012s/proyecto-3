//*****************************************************************************
// Universidad del Valle de Guatemala
// BE3015: Electrónica Digital 2
// Sofía Salguero - 19236
// Proyecto # 2
// Código para ESP 32
//*****************************************************************************

//*****************************************************************************
//Librerias
//*****************************************************************************
#include <Arduino.h>           //Librería de Arduino
#include <Wire.h>              // libreria para I2C
#include <Adafruit_BMP280.h>   //libreria para sensor
#include <Adafruit_NeoPixel.h> //libreria neopixel

//*****************************************************************************
//Definicion etiquetas
//*****************************************************************************

//Prescaler
#define prescaler 80

//Temporizadores
hw_timer_t *timer = NULL;

#define PIN 5
#define NUMPIXELS 1

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

Adafruit_BMP280 bmp;

//*****************************************************************************
//Prototipos de funcion
//*****************************************************************************
void IRAM_ATTR ISRTimer0();
void getTemperature(void);
void uart(void);
void configurarTimer(void);

//*****************************************************************************
//Varibles globales
//*****************************************************************************
int temperatura = 0; //Temperatura tomada por el sensor
int presion = 0;
int bandera = 1;             //bandera para comunicación UART TivaC con Esp32
boolean banderaUART = false; //bandera para comunicacion UART ESP32 con computadora

//*****************************************************************************
//ISR: interrupciones
//*****************************************************************************
void IRAM_ATTR ISRTimer0() //interrupción para timer
{
  banderaUART = true;
}

//*****************************************************************************
//Configuracion
//*****************************************************************************
void setup()
{
  //Configuración Serial
  Serial.begin(115200);
  Serial2.begin(115200);

  //Temporizadores
  configurarTimer();

  if (!bmp.begin(0x76))
  {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                     "try a different address!"));
    while (1)
      delay(10);
  }

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  pixels.begin();
  pixels.show();
}

//*****************************************************************************
//Loop principal
//*****************************************************************************
void loop()
{
  getTemperature(); //tomar distancia
  uart();           //UART con TivaC

  if (banderaUART == true) //Si ya pasaron 3 segundos, imprime la distancia
  {
    banderaUART = false;
    Serial.print("\nLa temperatura medida es de: ");
    Serial.print(temperatura);
    Serial.print(" °C");
    pixels.setPixelColor(0, pixels.Color(0, 0, 255));
  }

  pixels.show();
}

//******************************************************************************
// Configuración Timers
//******************************************************************************
void configurarTimer(void) //Timer para uart
{
  //Fosc = 80MHz = 80,000,000 Hz
  //Fosc / Prescaler = 80,000,000 / 80 = 1,000,000
  //Tosc = 1/Fosc = 1uS

  //Timer 0, prescaler = 80, flanco de subida
  timer = timerBegin(0, prescaler, true);

  //Handler de la interrupción
  timerAttachInterrupt(timer, &ISRTimer0, true);

  //Tic = 1uS    3s= 3000000uS
  timerAlarmWrite(timer, 3000000, true);

  //Inicia alarma
  timerAlarmEnable(timer);
}

//****************************************************************
// Conexión UART con Tiva C
//****************************************************************
void uart(void)
{
  while (Serial2.available() > 0) //Mira si hay algo en el buffer
  {
    bandera = Serial2.read();
    Serial2.write(temperatura); //Escribe en UART2 la distancia
  }
}

//*****************************************************************************
//Tomar temperatura y presion con el sensonr
//*****************************************************************************
void getTemperature(void)
{
  if (bandera == 0)
  {
    temperatura = bmp.readTemperature();
    presion = bmp.readPressure();
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
  }
}
