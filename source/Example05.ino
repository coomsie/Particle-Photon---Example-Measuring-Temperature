// -----------------------------------
// Example - 05: Measuring Temperature
// -----------------------------------
#include "application.h"
#include <math.h>
#include "TM1637.h"
#include "SparkFunPhant.h"
#include "HttpClient.h"
/*#include "thethingsiOClient.h"*/

const char server[] = "data.sparkfun.com"; // Phant destination server
const char publicKey[] = "JxLvQJ656JuJDm5gW9y3"; // Phant public key
const char privateKey[] = "gzJG1qwXwqfega7ndmXK"; // Phant private key
Phant phant(server, publicKey, privateKey); // Create a Phant object

/*thethingsiOClient thing("iDASYVfno4IwLrKy6G-Au2rRjddUhwiAPDJ-wFZ3P4I");*/

#define VARIABLE_ID "55ec2e357625420f56e10070"
#define TOKEN "xNKVRzWkBi0wFlsWhRARYQIC5kRlwT"

HttpClient http;

// Headers currently need to be set at init, useful for API keys etc.
http_header_t headers[] = {
    //  { "Content-Type", "application/json" },
    //  { "Accept" , "application/json" },
    { "Content-Type", "application/json" },
    { "X-Auth-Token" , TOKEN },
    // { "Accept" , "*/*"},
     { "Accept" , "application/json" },
    { NULL, NULL } // NOTE: Always terminate headers will NULL
};

http_request_t request;
http_response_t response;


void dispNum(unsigned int num);

// name the pins
#define CLK D4
#define DIO D5
TM1637 tm1637(CLK,DIO);
#define TEMPPIN A4


String Org = "Campion";
String Disp = "Campion";
String Locn = "Heathcote";

int lcd_start_pt = 4;

long previousMillis = 0;        // will store last time was updated

// the follow variables is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long interval = 30000;           // interval at which to send data to internet (milliseconds)

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

  //display startup message HEL0
  int8_t Disp[] = {0x01,0x02,0x03,0x04};
  Disp[0]=72;
  Disp[1]=14;
  Disp[2]=76;
  Disp[3]=48;
  tm1637.display(Disp);
  delay(1000);

}

// This routine loops forever
void loop()
{
  char payload[255];

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

  String tempC = "0";

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
    //ONLY PUBLISH EVERY X Time
    // difference between the current time and last time you blinked
      // the LED is bigger than the interval at which you want to
      // blink the LED.

      unsigned long currentMillis = millis();

      if(currentMillis - previousMillis > interval) {
        // save the last time you blinked the LED
        previousMillis = currentMillis;
        Spark.publish("temperature",tempC);

      ///snprintf(payload, sizeof(payload), "{ \"s\":\"wthr\", \"u\":\"F\",\"l\":\"%s\",\"m\":\"Temp\",\"o\":\"%s\",\"v\": %s,\"d\":\"%s\" }", Locn, Org,tempC, Disp);
      String azurePayload = "{ \"s\":\"wthr\", \"u\":\"C\",\"l\":\"" + Locn +"\",\"m\":\"Temp\",\"o\":\"" + Org + "\",\"v\": " + tempC + ",\"d\":\"" + Disp + "\" }";

      Spark.publish("ConnectTheDots", azurePayload);

      //publish to spark fun data hub
      postToPhant(tempC);

      // thing speak
      String _t = "{ \"t\" : " + tempC + "}";
      Spark.publish("ThingSpeak",_t);


      /*thing.addValue("temp",tempC);*/
      /*thing.send();*/

      //send via http client to ubidots
      // Request path and body can be set at runtime or at setup.
      request.hostname = "things.ubidots.com";
      request.port = 80;
      request.path = "/api/v1.6/variables/"VARIABLE_ID"/values";

      // The library also supports sending a body with your request:
      //request.body = "{\"key\":\"value\"}";

      // Send to Ubidots
      String resultstr;
      resultstr ="{\"value\": " + tempC + "}";

      Serial.print(" Ubidots : POST json\t" );
      Serial.println(resultstr);

      // Get request
      //http.get(request, response, headers);
      request.body = resultstr;
      http.post(request,response,headers);

      Serial.print("Application>\tResponse status: ");
      Serial.println(response.status);

      Serial.print("Application>\tHTTP Response Body: ");
      Serial.println(response.body);


    }
  }
};

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
  //make the positions all blank ...
  int lcdchars[] = {16,16,16,16};
  lcdchars[0] = num % 100 / 10;
  lcdchars[1] = num % 10;
  lcdchars[2] = 12;
  lcdchars[3] = 16;

  uint8_t i,count1 = 0;
  for(i=0;i<7;i++){
    //assign the positions...
    if(i == 0){
      TimeDisp[0] = lcdchars[0];
      TimeDisp[1] = lcdchars[1];
      TimeDisp[2] = lcdchars[2];
      TimeDisp[3] = lcdchars[3];
    }
    if(i == 1){
      TimeDisp[0] = 16;
      TimeDisp[1] = lcdchars[0];
      TimeDisp[2] = lcdchars[1];
      TimeDisp[3] = lcdchars[2];
    }
    if(i == 2){
      TimeDisp[0] = 16;
      TimeDisp[1] = 16;
      TimeDisp[2] = lcdchars[0];
      TimeDisp[3] = lcdchars[1];
    }
    if(i == 3){
      TimeDisp[0] = 16;
      TimeDisp[1] = 16;
      TimeDisp[2] = 16;
      TimeDisp[3] = lcdchars[0];
    }
    if(i == 4){
      TimeDisp[0] = 16;
      TimeDisp[1] = 16;
      TimeDisp[2] = 16;
      TimeDisp[3] = 16;
    }
    if(i == 5){
      TimeDisp[0] = lcdchars[2];
      TimeDisp[1] = lcdchars[3];
      TimeDisp[2] = 16;
      TimeDisp[3] = 16;
    }
    if(i == 6){
      TimeDisp[0] = lcdchars[1];
      TimeDisp[1] = lcdchars[2];
      TimeDisp[2] = lcdchars[3];
      TimeDisp[3] = 16;
    }
    /*tm1637.clearDisplay();*/
    tm1637.display(TimeDisp);
    delay(500);
  }

};


//POST TO SPARK FUN DATA HUB
int postToPhant(String tempC)
{
phant.add("temp", tempC );

TCPClient client;
char response[512];
int i = 0;
int retVal = 0;

if (client.connect(server, 80)) // Connect to the server
{
// Post message to indicate connect success
  Serial.println("Posting!");

// phant.post() will return a string formatted as an HTTP POST.
// It'll include all of the field/data values we added before.
// Use client.print() to send that string to the server.
  client.print(phant.post());
  delay(1000);
// Now we'll do some simple checking to see what (if any) response
// the server gives us.
  while (client.available())
  {
      char c = client.read();
      Serial.print(c);	// Print the response for debugging help.
      if (i < 512)
          response[i++] = c; // Add character to response string
  }
// Search the response string for "200 OK", if that's found the post
// succeeded.
  if (strstr(response, "200 OK"))
  {
      Serial.println("Post success!");
      retVal = 1;
  }
  else if (strstr(response, "400 Bad Request"))
  {	// "400 Bad Request" means the Phant POST was formatted incorrectly.
// This most commonly ocurrs because a field is either missing,
// duplicated, or misspelled.
      Serial.println("Bad request");
      retVal = -1;
  }
  else
  {
// Otherwise we got a response we weren't looking for.
      retVal = -2;
  }
}
else
{	// If the connection failed, print a message:
  Serial.println("connection failed");
  retVal = -3;
}
client.stop();	// Close the connection to server.
return retVal;	// Return error (or success) code.
};
