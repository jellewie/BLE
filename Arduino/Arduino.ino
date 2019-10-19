/*
   For the functions; SetPin, ReadPin, Disconnect, ConnectTo, CheckSerialBLE, CheckSerialPC, HandleSerialDataPC, Delay, SendToBLE, etc see 'Functions'

   do NOT use 'delay()' use 'Delay()', the Arduino 'delay()' will freeze the chip, and will mess up serial comminication. the custom 'Delay()' wont
*/
const int MaxAmountOfSlaves = 20;
String SlaveID[MaxAmountOfSlaves] = {"508CB174C9B6", "20C38FBE38AC", "D43639716B15"};      //The MAC id of the slave(s)
bool Pinstates[MaxAmountOfSlaves];
byte PDI_ProgSlave = 4;                               //Pin with the button to start flashing slave
byte PDI_DipSwitch[8] = {2, 2, 2, 2, 2, 2, 2, 2};     //Pins where the DIP switch is connected to
byte PDO_LEDS[8] = {3, 3, 3, 3, 3, 3, 3, 3};          //Pins where the LEDS are connected to


bool Automode = false;
unsigned long EveryXms = 10000;                       //Time in ms to execute BLE commands

//===============
#define ShowComData                                   //Enable this to show all debug data send and recieved from BLE
String SerialData = "";                               //The Last serieal data
bool CurentlyConnected;                               //If the Arduino 'knows' it it's connected

void setup() {                                        //This is runned once on bootup
  Serial.begin(115200);                               //Start Serial connection PC
  Serial1.begin(115200);                              //Start Serial connection BLE

  for (byte i = 0; i < sizeof(PDI_DipSwitch); i++)     //For all Dip switches
    pinMode(PDI_DipSwitch[i], INPUT);                 //Set each as input
  for (byte i = 0; i < sizeof(PDO_LEDS); i++)          //For all LEDs
    pinMode(PDO_LEDS[i], OUTPUT);                     //Set each as input

  Serial.println("Booted");
  if (Disconnect())                                   //Make sure we aredisconnected
    Serial.println("Succesfull disconnected");
}
void loop() {                                         //After start up this will be run repearetly
  CheckSerialPC();                                    //Check and Get PC com data
  CheckSerialBLE();                                   //Check and Get BLE com data
  static unsigned long LastTime;                      //Create a value that stores the last updated time (only once, will keep it's data after the loop since it's static)
  if (millis() > LastTime + EveryXms and Automode) {  //If it's time to update
    LastTime = LastTime + EveryXms;                   //Set new LastTime updated (This is not set to now, Since we want to loop the code every EveryXms, no wait EveryXms after the code)
    DoAnUpdate();                                     //Send and recieve commands of all slaves
  }

  static bool SlaveProcessed;
  if (!SlaveProcessed and digitalRead(PDI_ProgSlave)) {  //Check if A slave has been added that needs to be programmed
    SlaveProcessed = true;
    Delay(1000);   //Wait a bit to make sure it's properly connected
    Serial3.begin(115200);                              //BLE slave
    //HandleSerialDataPC("SomeATCommandsToSend", false);
    Serial3.end();
  } else {
    SlaveProcessed = false;
  }
}
void DoAnUpdate() {                                   //This is your custom code
  //===============for each slave
  for (byte i = 0; i < (sizeof(SlaveID) / sizeof(String)); i++) {
    Serial.println("_" + String(i));
    //===============Connect
    if (ConnectTo(SlaveID[i])) {
      Serial.println("Succesfull connected to " + String(SlaveID[i]));
      Delay(10);
      //===============Read
      String PinValue = ReadPin("B");
      if (PinValue != "")
        Serial.println("Pin B of slave " + String(SlaveID[i]) + " = " + PinValue);
    }
    //===============disconnect
    if (Disconnect())
      Serial.println("Succesfull disconnected");
    //===============Small delay
    Delay(150); //Add some delay before sending next command, the master needs to be ready to connect again
    //===============
  }
}
void PinChanged(byte pin, byte state) {               //This code is called when a BLE module send a pin update state
  //We only get updates when a pin is high. So when all are low we won't recieve this (Altough next update I do send a low update if posible). (update speed = AT+CYC## in sec)
  Serial.println("PinChanged " + String(pin) + " to " + String(state));
}
void SerialDebugCommands(String Data) {               //Function to listen to custom Serial commands from the PC

}
