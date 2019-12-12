#define ShowComData

String SlaveID[] = {"508CB174C9B6", "20C38FBE38AC", "D43639716B15"}; //The MAC id of the slave(s)
const int AmountOfSlaves = sizeof(SlaveID) / sizeof(String);

const byte AmountOfMasters = 3;
bool CurentlyConnected[AmountOfMasters];  //If we are succesfully connected or not
byte LastPinHEX[AmountOfMasters];      //Last known HEX states of the slave
byte Mode[AmountOfMasters];               //Current mode of the masters
String CurentlyMac[AmountOfMasters];      //To what slave each master is connected to
static byte TempModeValue[AmountOfMasters]; //A slot to store a value though lopes per master, Like pin state 'HIGH'
unsigned long ModeTimeOutAt[AmountOfMasters];
//Mode 0=nothing
#define ModeDisconnect 1
#define ModeConnect 2
#define ModeReadpin 3
#define ModeSetPin 4

void setup() {
  Serial.begin(115200);                                             //Start Serial connection PC
  Serial1.begin(115200);                                            //Start Serial connection BLE Master 1
  Serial2.begin(115200);                                            //Start Serial connection BLE Master 2
  Serial3.begin(115200);                                            //Start Serial connection BLE Master 3
}

void loop() {
  String TempRecieved;
  for (byte i = 0; i < AmountOfMasters; i++) {    //For each master
    TempRecieved = CheckSerial(i);
    if (TempRecieved != "") {
      ModeCheck(TempRecieved, i);           //TempRecieved = new data to process
    }
    if (ModeTimeOutAt[i] > 0) {              //If we have a time-out timer
      if (millis() > ModeTimeOutAt[i]) {     //If time-outed
        if (Mode[i] == ModeDisconnect)
          Serial.println("Disconnect Timed-Out");
        if (Mode[i] == ModeConnect)
          Serial.println("Connect Timed-Out");
        if (Mode[i] == ModeReadpin)
          Serial.println("ReadPin Timed-Out");
        if (Mode[i] == ModeSetPin)
          Serial.println("SetPin Timed-Out");
        ModeTimeOutAt[i] = 0;
        Mode[i] = 0;
      }
    }
  }
  DoAnUpdate();
  yield();
  delay(1);
}


void DoAnUpdate() { //RUNS AT CLOCK SPEED!
  //your code

  for (byte i = 0; i < AmountOfMasters; i++) {    //For each master
    if (Mode[i] == 0) {                           //if doing nothing
      if (!CurentlyConnected[i]) {                //if not connected
        ConnectTo(SlaveID[i], i);                 //connect
      }
    }
  }
}




void PinChanged(byte pin, byte state, byte MasterID) {                             //This code is called when a BLE module send a pin update state
  //We only get updates when a pin is high. So when all are low we won't recieve this (Altough next update I do send a low update if posible). (update speed = AT+CYC## in sec)
#ifdef ShowComData
  Serial.println("PinChanged " + String(pin) + " to " + String(state) + " MasterID " + String(MasterID));
#endif //ShowComData
}
