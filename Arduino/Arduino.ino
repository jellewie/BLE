/*
   For the functions; SetPin, ReadPin, Disconnect, ConnectTo, CheckSerialBLE, CheckSerialPC, HandleSerialDataPC, Delay, SendToBLE, etc see 'Functions'

   do NOT use 'delay()' use 'Delay()', the Arduino 'delay()' will freeze the chip, and will mess up serial comminication. the custom 'Delay()' wont
*/
const int MaxAmountOfSlaves = 20;
String SlaveID[MaxAmountOfSlaves] = {"508CB174C9B6", "20C38FBE38AC", "D43639716B15"};      //The MAC id of the slave(s)
bool Automode = false;
unsigned long EveryXms = 10000;                       //Time in ms to execute BLE commands
byte AmountOfSlaves;                                  //Amount of slaves
byte SelectedSlave;                                   //Selecter to wich slave we are talking

bool Pinstates[MaxAmountOfSlaves];
const static byte PDI_DipSwitch[8] = {2, 2, 2, 2, 2, 2, 2, 2}; //Pins where the DIP switch is connected to
const static byte PDO_LEDS[8] = {3, 3, 3, 3, 3, 3, 3, 3};//Pins where the LEDS are connected to

//===============
#define ShowComData                                   //Enable this to show all debug data send and recieved from BLE
String SerialData = "";                               //The Last serieal data
bool CurentlyConnected;                               //If the Arduino 'knows' it it's connected

void setup() {                                        //This is runned once on bootup
  Serial.begin(115200);                               //Start Serial connection PC
  Serial1.begin(115200);                              //Start Serial connection BLE

  for (int i = 0; i < sizeof(PDI_DipSwitch); i++)     //For all Dip switches
    pinMode(PDI_DipSwitch[i], INPUT);                 //Set each as input
  for (int i = 0; i < sizeof(PDO_LEDS); i++)          //For all LEDs
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
  if (false and !SlaveProcessed) {  //Check if A slave has been added that needs to be programmed
    SlaveProcessed = true;
    Delay(1000);   //Wait a bit to make sure it's properly connected
    Serial3.begin(115200);                              //BLE
    //HandleSerialDataPC("SomeATCommandsToSend");
    Serial3.end();
  } else {
    SlaveProcessed = true;
  }
}
void DoAnUpdate() {                                   //This is your custom code
  //===============for each slave
  for (int i = 0; i < AmountOfSlaves; i++) {
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
  //This function can be cleared, uncommented (or removed) it's just here to allow me to quickly debug functions
  /*Commands:
    ADD<MAC>    Add a MAC id to the slave list
    REM<MAC>    Remove a MAC adres
    LIS         List of all slaves
    SPE         Poll speed in ms to get data from the slaves
    SAV         Save settings
    PRO         (Re)program master
    TES         Test all pins
    UPD         Force an update from all salves
  */
  Data.toUpperCase();
  Data.trim();
  if (SerialData.length() >= 3) {
    if (Data.substring(0, 3) == "ADD") {
      if (SerialData.length() >= 15) {
        String MAC = Data.substring(4, 15); //20C38FBE38AC
        if ((sizeof(SlaveID) / sizeof(String)) == MaxAmountOfSlaves) {
          Serial.println("Max amount of slaves reached");
        } else {
          SlaveID[AmountOfSlaves + 1] = MAC;
          Serial.println("Add this mac" + MAC);
        }
      } else {
        Serial.println("Please add 12 digit MAC ID");
      }
    } else if (Data.substring(0, 3) == "REM") {
      if (SerialData.length() >= 15) {
        String MAC = Data.substring(4, 15); //20C38FBE38AC
        if ((sizeof(SlaveID) / sizeof(String)) == MaxAmountOfSlaves) {
          Serial.println("Max amount of slaves reached");
        } else {
          Serial.println("Remove this mac (FUNCTION NOT YET IMPLENTED :(" + MAC);
        }
      } else {
        Serial.println("Please add 12 digit MAC ID");
      }
    } else if (Data.substring(0, 3) == "LIS") {
      for (int i = 0; i < AmountOfSlaves; i++) {
        Serial.println("Slave at" + String(i) + " has mac '" + SlaveID[i] + "'");
      }
    } else if (Data.substring(0, 3) == "SPE") {
      if (SerialData.length() >= 4) {
        int Temp = Data.substring(4).toInt();
        EveryXms = Temp;
        Serial.println("Change update speed to every '" + String(Temp) + "'ms ");
      }
    } else if (Data.substring(0, 3) == "SAV") {//Save
      Serial.println("Save settings (FUNCTION NOT YET IMPLENTED :(");

    } else if (Data.substring(0, 3) == "PRO") {//Re-program Master
      Serial.println("(Re)program master (FUNCTION NOT YET IMPLENTED :(");
      //HandleSerialDataPC("SomeATCommandsToSend");

    } else if (Data.substring(0, 3) == "TES") {
      Serial.println("Test command (FUNCTION NOT YET IMPLENTED :(");

    } else if (Data.substring(0, 3) == "UPD") {
      Serial.println("Forcing update of all slaves...");
      DoAnUpdate();
    }
  }

  //Some debug stuff
  if (Data == "D") {
    Disconnect();
  } else if (Data.substring(0, 1) == "R") {
    String pin = "B";
    if (Data.length() >= 2)
      pin = Data.substring(1, 2);
    String Back = ReadPin(pin);
    Serial.println(String(Back) + "(Emthy = failed)");
  } else if (Data.substring(0, 1) == "S") {
    byte pin = 2;
    if (Data.length() >= 2)
      pin = Data.substring(1, 2).toInt();
    byte state = 1;
    if (Data.length() >= 3)
      state = Data.substring(2, 3).toInt();
    bool Back = SetPin(pin, state);
    Serial.println(String(Back) + "(1=YES)");
  } else if (Data.substring(0, 1) == "C") {
    byte i = 0;
    if (Data.length() >= 2)
      i = Data.substring(1, 2).toInt();
    bool Back = ConnectTo(SlaveID[i]);
    Serial.println(String(Back) + "(1=YES) connect to ID " + String(SlaveID[i]));
  } else if (Data.substring(0, 1) == "M") {
    Automode = !Automode;
    //Since this if for debug, I don't fix the 'trying to catch up' behavior after turning this on
    Serial.println("Automode set to " + String(Automode));
  }
}
