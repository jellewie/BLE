/*
   For the functions; SetPin, ReadPin, Disconnect, ConnectTo, CheckSerialBLE, CheckSerialPC, HandleSerialData, Delay, SendToBLE, etc see 'Functions'

   do NOT use 'delay()' use 'Delay()', the Arduino 'delay()' will freeze the chip, and will mess up serial comminication. the custom 'Delay()' wont

   TODO:
      ADD THE SLAVE AT COMMANDS

*/
const int AmountOfSlaves = 3;
String SlaveID[AmountOfSlaves] = {"508CB174C9B6", "20C38FBE38AC", "D43639716B15"}; //The MAC id of the slave(s)
const byte ColorHIG[] {0  , 255, 0  };                              //RGB color HIGH/ON
const byte ColorLOW[] {255, 30 , 0  };                              //RGB color LOW/off
const byte ColorDIS[] {255, 0  , 0  };                              //RGB color disconnected
const byte ColorCON[] {0  , 0  , 0  };                              //RGB color while connecting
const byte ColorUNK[] {0  , 0  , 255};                              //RGB color when we where unable to read the pin

//===============
#define ShowComData                                                 //Enable this to show all debug data send and recieved from BLE
const int TimeOutMaster = 150;                                      //Time for the master to rest between disconnect and connect to next slave
//120 gives quite some errors, 150ms seems to be like 99%> good
const byte PDI_UpdateStates = 11;                                   //Pin where the manual update button is located
const byte PDI_ProgSlave = 10;                                      //Pin with the button to start flashing slave
const byte PDI_DipSwitch[] = {2, 3, 4, 5, 6, 7, 8, 9};              //Pins where the DIP switch is connected to
const byte PAO_LED = 12;                                            //Where the <LED strip> is connected to
const byte PDO_LED_BUILTIN = 13;                                    //This is the Arduino LED (better known as LED_BUILTIN, but set here manually)
const byte PDO_Outputs[] = {22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};
#include <FastLED.h>                                                //Include the FastLED library to control the LEDs in a simple fashion
CRGB LEDs[AmountOfSlaves];                                          //This is an array of LEDs. One item for each LED in your strip.
unsigned long EveryXms;                                             //Time in ms of pull rate BLE (0 to send then on startup)
//===============
String SerialData = "";                                             //The Last serieal data
bool CurentlyConnected;                                             //If the Arduino 'knows' it it's connected

void setup() {                                                      //This is runned once on bootup
  Serial.begin(115200);                                             //Start Serial connection PC
  Serial1.begin(115200);                                            //Start Serial connection BLE

  for (byte i = 0; i < sizeof(PDI_DipSwitch); i++)                  //For all Dip switches
    pinMode(PDI_DipSwitch[i], INPUT_PULLUP);                        //Set each as input
  pinMode(PDI_UpdateStates,   INPUT_PULLUP);                        //Set each as input
  pinMode(PDI_ProgSlave,      INPUT_PULLUP);                        //Set each as input
  pinMode(PAO_LED,            OUTPUT);                              //Sometimes the Arduino needs to know what pins are OUTPUT and what are INPUT,
  for (byte i = 0; i < sizeof(PDO_Outputs); i++)                    //For all output LEDS
    pinMode(PDO_Outputs[i],   OUTPUT);                              //Set each as output
  pinMode(PDO_LED_BUILTIN,    OUTPUT);                               //

  FastLED.addLeds<WS2812B, PAO_LED, GRB>(LEDs, AmountOfSlaves);     //Set the LED type and such
  fill_solid(&(LEDs[0]), AmountOfSlaves, CRGB(1, 1, 1));            //Set the whole LED strip to be on slightly
  FastLED.show();                                                   //Update
  FastLED.setBrightness(16);                                        //Scale brightness
  delay(500);                                                       //Wait a bit to show LEDs are working
  fill_solid(&(LEDs[0]), AmountOfSlaves, CRGB(0, 0, 0));            //Set the whole LED strip to be off (Black)
  FastLED.show();                                                   //Update

  Serial.println("Booted");
  if (Disconnect())                                                 //Make sure we aredisconnected
    Serial.println("Succesfull disconnected");
}
void loop() {                                                       //After start up this will be run repearetly
  CheckSerialPC();                                                  //Check and Get PC com data
  CheckSerialBLE(true);                                                 //Check and Get BLE com data

  //  static unsigned long LastTime;                                    //Create a value that stores the last updated time (only once, will keep it's data after the loop since it's static)
  //  if (millis() > LastTime + EveryXms) {                             //If it's time to update
  //    LastTime = millis();                                            //Set new LastTime updated (This will prospone the update for 'EveryXms' amount of time)
  //    DoAnUpdate();                                                   //Send and recieve commands of all slaves
  //    SetEveryXms();
  //  }

#define EveryXmsBlink 500
  static unsigned long LastTimeBlink;                               //Create a value that stores the last updated time
  if (millis() > LastTimeBlink + EveryXmsBlink) {                   //If it's time to update
    LastTimeBlink = millis();                                       //Set new LastTime updated
    digitalWrite(PDO_LED_BUILTIN, !digitalRead(PDO_LED_BUILTIN));   //Let the LED blink
  }

  static bool DoneSlaveProgrammed;
  if (digitalRead(PDI_ProgSlave) == LOW) { //Check if A slave has been added that needs to be programmed
    if (!DoneSlaveProgrammed) {
      DoneSlaveProgrammed = true;
      Serial2.begin(115200);                                        //Start BLE slave comminication
      HandleSerialData("AT+RENEWAT+MODE1AT+PASS191019AT+AD1508CB149790AAT+NAMEBLEslave", false);
      HandleSerialData("AT+ALLO1AT+PWRM0AT+POWE3AT+RESET", false);
      Serial.println("done programming slave");
      Serial2.end();
    }
  } else {
    DoneSlaveProgrammed = false;
  }

  if (digitalRead(PDI_UpdateStates) == LOW)
    DoAnUpdate();
}
void DoAnUpdate() {                                                 //This is your custom code
  for (byte i = 0; i < AmountOfSlaves; i++) {                       //for each slave
    LEDs[i] = CRGB(ColorCON[0], ColorCON[1], ColorCON[2]);          //Set the connecting color
    FastLED.show();                                                 //Update the LED's
    if (ConnectTo(SlaveID[i])) {                                    //Connect
      if (SetPin(2, 1)) {                                           //Set the power to the switch
        String TempPinValue = ReadPin("B");                         //Read the switch pin
        if (TempPinValue != "") {                                   //If we have read something
          float TempValue = TempPinValue.toFloat();                 //Convert it to a float
          Serial.println("Pin B of slave " + String(SlaveID[i]) + " ID " + String(i) + " = " + TempValue);
          if (TempValue > 3) {                                      //if it's above this amount of Volts
            digitalWrite(PDO_Outputs[i], HIGH);                     //Set the output pin
            LEDs[i] = CRGB(ColorHIG[0], ColorHIG[1], ColorHIG[2]);  //Show pin high color
          } else {
            digitalWrite(PDO_Outputs[i], LOW);                      //Set the output pin
            LEDs[i] = CRGB(ColorLOW[0], ColorLOW[1], ColorLOW[2]);  //Show pin low color
          }
          FastLED.show();                                           //Update the LED's
          if (!SetPin(2, 0))                                        //remove the power from the switch
            Serial.println("could not set pin low again");
        }
      } else {
        LEDs[i] = CRGB(ColorUNK[0], ColorUNK[1], ColorUNK[2]);      //Set the unknown status
        digitalWrite(PDO_Outputs[i], LOW);                          //Set the output pin
        FastLED.show();                                             //Update the LED's
      }
      if (!CurentlyConnected) {                                     //If the device disconnected itzelf
        Serial.println("The device disconnected itzelf, Retry...");
        i--;                                                        //Retry this device again
      }
    } else {
      Serial.println("Could not connect to " + SlaveID[i]);
      LEDs[i] = CRGB(ColorDIS[0], ColorDIS[1], ColorDIS[2]);        //Show disconnected color
      FastLED.show();
    }
    if (Disconnect())
      Delay(TimeOutMaster);                                         //Add some delay before sending next command, the master needs to be ready to connect again
  }
}
void PinChanged(byte pin, byte state) {                             //This code is called when a BLE module send a pin update state
  //We only get updates when a pin is high. So when all are low we won't recieve this (Altough next update I do send a low update if posible). (update speed = AT+CYC## in sec)
  Serial.println("PinChanged " + String(pin) + " to " + String(state));
}
void SerialDebugCommands(String Data) {                             //Function to listen to custom Serial commands from the PC

}
void SetEveryXms() {
  //The time set with the DipSwitches (min 1 sec, max 12 Hours)
  int TempDelay = 1;
  if (digitalRead(PDI_DipSwitch[0]) == LOW)
    TempDelay += TempDelay * 10;
  if (digitalRead(PDI_DipSwitch[1]) == LOW)
    TempDelay += TempDelay * 30;
  if (digitalRead(PDI_DipSwitch[2]) == LOW)
    TempDelay += TempDelay * 60;
  if (digitalRead(PDI_DipSwitch[3]) == LOW)
    TempDelay += TempDelay * 120;
  if (digitalRead(PDI_DipSwitch[4]) == LOW)
    TempDelay += TempDelay * 240;
  if (digitalRead(PDI_DipSwitch[5]) == LOW)
    TempDelay += TempDelay * 480;
  if (digitalRead(PDI_DipSwitch[6]) == LOW)
    TempDelay += TempDelay * 960;
  if (digitalRead(PDI_DipSwitch[7]) == LOW)
    TempDelay += TempDelay * 1920;
  EveryXms = TempDelay * 1000;                 //Set and convert from s to ms
}
