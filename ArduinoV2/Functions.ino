void ConnectTo(String Mac, byte MasterID) {
  CurentlyConnected[MasterID] = false;    //just in case
  SendSerial("AT+CON" + Mac, MasterID);
  ChangeMode(MasterID, ModeConnect, 11000);
  CurentlyMac[MasterID] = Mac;      //Mark the slave to this MasterID
}
void Disconnect(byte MasterID) {
  SendSerial("AT", MasterID);
  CurentlyConnected[MasterID] = false;    //just in case
  ChangeMode(MasterID, ModeDisconnect, 500);
}
void SetPin(String pin, byte state, byte MasterID) {
  SendSerial("AT+PIO" + String(pin) + String(state), MasterID);
  ChangeMode(MasterID, ModeSetPin, 1000);
  TempModeValue[MasterID] = state;
}
void ReadPin(String pin, byte MasterID) {
  //Will call 'PinChanged' with the answer
  SendSerial("AT+ADC" + pin + "?", MasterID);
  ChangeMode(MasterID, ModeReadpin, 1000);
}
void ChangeMode(byte MasterID, byte ModeSetTo, int TimeMS) {
  Mode[MasterID] = ModeSetTo;
  ModeTimeOutAt[MasterID] = millis() + TimeMS;
}
void ModeCheck(String Data, byte MasterID) {
  if (Mode[MasterID] == ModeReadpin) {
    if (Data.length() >= 8)
      if (Data.substring(0, 6) == "OK+ADC") {
        PinChanged(Data.substring(7).toInt(), Data.substring(8).toInt(), MasterID);
        Mode[MasterID] = 0;                       //Flag we are done
      }
  } else if (Mode[MasterID] == ModeSetPin) {             //If we where in the right mode
    if (Data.length() >= 8) {
      if (Data.substring(0, 6) == "OK+PIO") {
        if (TempModeValue[MasterID] != Data.substring(8).toInt())
          Serial.println("SetPin failed '" + String(TempModeValue[MasterID]) + "' but in '" + Data.substring(8) + "'");
        Mode[MasterID] = 0;                       //Flag we are done
      }
    }
  }
}
void SendSerial(String Text, byte MasterID) {

#ifdef ShowComData
  Serial.println("<= '" + String(MasterID) + "'_'" + Text + "'");
#endif //ShowComData
  if (MasterID == 0) {
    Serial.print(Text);
  } else if (MasterID == 1) {
    Serial1.print(Text);
  } else if (MasterID == 2) {
    Serial2.print(Text);
  } else if (MasterID == 3) {
    Serial3.print(Text);
  }
}
String CheckSerial(byte MasterID) {
  String TempSerialData = "";
  if (MasterID == 0) {
    while (Serial.available()) {
      delay(2);
      TempSerialData += char(Serial.read());
    }
  } else if (MasterID == 1) {
    while (Serial1.available()) {
      delay(2);
      TempSerialData += char(Serial1.read());
    }
  } else if (MasterID == 2) {
    while (Serial2.available()) {
      delay(2);
      TempSerialData += char(Serial2.read());
    }
  } else if (MasterID == 3) {
    while (Serial3.available()) {
      delay(2);
      TempSerialData += char(Serial3.read());
    }
  }
  if (TempSerialData != "") {
    TempSerialData.trim();                                      //Remove spaces
    TempSerialData.replace(String(char(10)), "");               //Remove /n
    TempSerialData.replace(String(char(13)), "");               //Remove /r

    if (TempSerialData == "OK+CONN") {                      //Connected
      CurentlyConnected[MasterID] = true;                       //Flag we are connected
      if (Mode[MasterID] == ModeConnect)                        //If we where in the right mode
        ModeTimeOutAt[MasterID] = 0;                            //Stop (reset) Time-out timer
    } else if (TempSerialData == "OK+LOST") {               //Disconnected
      CurentlyConnected[MasterID] = false;
      if (Mode[MasterID] == ModeDisconnect)                     //If we where in the right mode
        ModeTimeOutAt[MasterID] = 0;                            //Stop (reset) Time-out timer
    } else if (TempSerialData.length() >= 9) {              //PinChange 'OK+Col:01'
      if (TempSerialData.substring(0, 6) == "OK+Col") {
        byte TempPinHEX = TempSerialData.substring(7, 9).toInt();   //Get current bit status
        byte ChangedPins = ~(TempPinHEX ^ (~LastPinHEX[MasterID])); //Calculate all bits that are updated (Bitwise Xor + inverse)
        LastPinHEX[MasterID] = TempPinHEX;                      //Save status so we can calculate updated bits next time
        for (int i = 0; i <= 8; i++) {                          //For each bit
          if (bitRead(ChangedPins, i))                          //If this bit is not the same as before
            PinChanged(11 - i, bitRead(TempPinHEX, i), MasterID);
        }
      }
    }
#ifdef ShowComData
    Serial.println("=> '" + String(MasterID) + "'_'" + TempSerialData + "'");
#endif //ShowComData
  }
  return TempSerialData;
}
