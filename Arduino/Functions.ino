bool SetPin(byte pin, byte state) {
  //AT+PIO21
  SendToBLE("AT+PIO" + String(pin) + String(state));
  unsigned long StopAt = millis() + 1000; //(440ms measured as time responce)
  while (millis() < StopAt) {
    CheckSerialBLE();
    if (SerialData.length() >= 8) {
      if (SerialData.substring(0, 6) == "OK+PIO") {
        if (String(state) == SerialData.substring(8))
          return true;
        else {
          Serial.println("SetPin failed '" + String(state) + "' but in '" + SerialData.substring(8) + "'");
          return false;
        }
      }
    }
  }
  Serial.println("SetPin Timed-Out");
  return false;
}
String ReadPin(String pin) {
  //Only works for pins 4,5,6,7,8,9,A,B
  SendToBLE("AT+ADC" + pin + "?");
  unsigned long StopAt = millis() + 1000; //(440ms measured as time responce)
  while (millis() < StopAt) {
    CheckSerialBLE();
    if (SerialData.length() >= 8)
      if (SerialData.substring(0, 6) == "OK+ADC")
        return SerialData.substring(8);
  }
  Serial.println("ReadPin Timed-Out");
  return "";
}
bool Disconnect() {
  //returns true when it succesful disconnected from something
  SendToBLE("AT");
  CurentlyConnected = false;
  unsigned long StopAt = millis() + 1000;
  while (millis() < StopAt) {
    CheckSerialBLE();
    if (SerialData == "OK+LOST")
      return true;
    else if (SerialData == "OK")
      return false;
  }
  Serial.println("Disconnect Timed-Out");
  return false;
}
bool ConnectTo(String Mac) {
  //returns true when it's (alread) connected
  //Will timeout if already the BLE is already conencted!
  CurentlyConnected = false;
  SendToBLE("AT+CON" + Mac);
  unsigned long StopAt = millis() + 11000;
  while (millis() < StopAt) {
    CheckSerialBLE();
    if (SerialData == "OK+CONN")
      return true;
    else if (SerialData == "OK+CONNE" or SerialData == "OK+CONNF")
      return false; //Fault or Error detected
  }
  Serial.println("ConnectTo Timed-Out; can't find '" + Mac + "'");
  return false;
}
void CheckSerialBLE() {
  SerialData = "";
  if (Serial1.available()) {
    while (Serial1.available()) {
      delay(2);
      SerialData += char(Serial1.read());
    }
    SerialData.trim();
    if (SerialData != "")
#ifdef ShowComData
      Serial.println("=> '" + SerialData + "'");
#endif //ShowComData
    if (SerialData == "OK+CONN")                    //Connected 'OK+CONN'
      CurentlyConnected = true;
    if (SerialData.length() >= 9)                   //PinChange 'OK+Col:01'
      if (SerialData.substring(0, 6) == "OK+Col") {
        static byte LastPinHEX = 0;
        byte PinHEX = SerialData.substring(7, 9).toInt();   //Get current bit status
        byte ChangedPins = ~(PinHEX ^ (~LastPinHEX));     //Calculate all bits that are updated (Bitwise Xor + inverse)
        LastPinHEX = PinHEX;                              //Save status so we can calculate updated bits next time
        for (int i = 0; i <= 8; i++) {                    //For each bit
          if (bitRead(ChangedPins, i))                    //If this bit is not the same as before
            PinChanged(11 - i, bitRead(PinHEX, i));
        }
      }
  }
}

void CheckSerialPC() {
  if (Serial.available()) {
    String Temp = "";
    while (Serial.available()) {
      delay(2);
      Temp += char(Serial.read());
    }
    HandleSerialDataPC(Temp);
  }
}
void HandleSerialDataPC(String SerialDataFeedback) {
  //Trying to make a command splitter, so we could send mote commands at once
  int CMDStartAt = 0, CMDAmount = 0;
  String oneLine = SerialDataFeedback, sa[100];
  for (unsigned int i = 0; i < oneLine.length(); i++) {
    if (oneLine.substring(i, i + 3) == "AT+") {
      if (i > 0) {
        sa[CMDAmount] = oneLine.substring(CMDStartAt - 1, i);
        CMDAmount++;
      }
      CMDStartAt = (i + 1);
    }
  }
  if (CMDAmount == 0) {       //No AT+ command, send all data just raw
    sa[CMDAmount] = SerialDataFeedback;
    CMDAmount++;
  } else {                    //Add last AT+ command
    sa[CMDAmount] = oneLine.substring(CMDStartAt - 1);
    CMDAmount++;
  }
  for (int i = 0; i < CMDAmount; i++) {
    SendToBLE(sa[i]);
    SerialDebugCommands(sa[i]);
    if (CMDAmount > 1) {
      unsigned long StopAt = millis() + 1000;
      while (millis() < StopAt and SerialData == "") {
        CheckSerialPC();  //PC
        CheckSerialBLE(); //BLE
      }
      Delay(10);
      Serial.println("___Next MultiLine CMD___");
    }
  }
}
void Delay(int AmountOfMs) {
  //This will just add a delay to the code, but still keep the Serial connection going.
  unsigned long StopAt = millis() + AmountOfMs;
  while (millis() < StopAt) {
    CheckSerialPC();  //PC
    CheckSerialBLE(); //BLE
  }
}
void SendToBLE(String Text) {
#ifdef ShowComData
  Serial.println("<= '" + Text + "'");
#endif //ShowComData
  Serial1.print(Text);
}