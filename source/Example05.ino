// -----------------------------------
// Example - 05: Measuring Temperature
// -----------------------------------
#include "application.h"
#include <math.h>
#include "TM1637.h"

void dispNum(unsigned int num);

// name the pins
#define CLK D4
#define DIO D5
TM1637 tm1637(CLK,DIO);
#define TEMPPIN A4



///FOR TESTING DIGIT display
String inputString = ""; // a string to hold incoming data
char x1 = 0; // char to attribute to 2nd digit of the display

bool debugMode = false;

// This routine runs only once upon reset
void setup()
{
  tm1637.set();                                 // cofig TM1637
  tm1637.init();                                // clear the display
  Serial.begin(9600);                           // init serial port on USB interface
}

// This routine loops forever
void loop()
{
  //for debuging the LED
  if (Serial.available() > 0) {
    // get the new byte:
    char inChar = (char)Serial.read();

    // add it to the inputString:
    inputString += inChar;

    // if the incoming character is a newline,
    if (inChar == '\n') {
    // if the incoming character is a newline,
      Serial.print("string ");
      Serial.println(inputString); //print string

      x1 = inputString[1]; // attach x1 to the second char in the string created
      Serial.print("digit ");
      Serial.println(x1); // print that second char to check if everything is ok
      tm1637.display(2,x1-'0');// display the char attached to x1 on the second digit
      tm1637.display(3,12);
      tm1637.display(0,14);
      inputString = "";
  }
}


  int B = 3975;                                 // B value of the thermistor

  String tempC = "0 C";

  int analogValue = analogRead(TEMPPIN);        // read rotary pin
  float resistance=(float)(4095-analogValue)*10000/analogValue;   //get the resistance of the sensor
  float temperature=1/(log(resistance/10000)/B+1/298.15)-273.15;  //convert to temperature via datasheet
  if (debugMode){
  Serial.print("analogValue: ");
  Serial.println(analogValue);
  Serial.print("resistance: ");
  Serial.println(resistance);
  Serial.print("temperature: ");
  Serial.println(temperature);
  Serial.println("");
}
  dispNum((unsigned int) (temperature + 0.5));  // display the voltage
  tempC = ((String) (temperature + 0.5));
  //// if connected to cloud then
  if (Spark.connected()){
  Spark.publish("temperature",tempC);
  }
  delay(2000);
}

// display a integer value less then 10000
void dispNum(unsigned int num)
{
  int8_t TimeDisp[] = {0x01,0x02,0x03,0x04};    // limit the maximum number
  tm1637.set(BRIGHT_TYPICAL);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
  if(num > 9999) num = 9999;
  /*TimeDisp[0] = num / 1000;
  TimeDisp[1] = num % 1000 / 100;
  TimeDisp[2] = num % 100 / 10;
  TimeDisp[3] = num % 10;
  */
  TimeDisp[0] = num % 100 / 10;
  TimeDisp[1] = num % 10;
  TimeDisp[2] = 12;
  TimeDisp[3] = 16;

  tm1637.clearDisplay();
  delay(500);
  tm1637.display(TimeDisp);
}
