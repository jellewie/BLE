//===============
//EXAMPLES of codes to put in DoAnUpdate
//===============
//Check if connected, else connect
//===============
//static byte SelectedSlave = 0;
//if (!CurentlyConnected) {
//  if (Disconnect()) {                         //We should be disconnected, so tell the BLE module this
//    Serial.println("Succesfull disconnect");
//    delay(200);                               //The BLE module was connected, so wait for shutdown before reboot
//  }
//  if (ConnectTo(SlaveID[SelectedSlave])) {
//    Serial.println("Succesfull connected to " + String(SlaveID[SelectedSlave]));   //NOT re-connected, this is auto done if posible
//    delay(100);                               //Wait for boot
//  }
//  else
//    LastTime += 10000;                        //Timeout 10 sec (no BLE was found)
//}

//===============
//EXAMPLE Read a pin state
//===============
//if (CurentlyConnected) {
//  String PinValue = ReadPin("B");
//  if (PinValue != "")
//    Serial.println("Pin B = " + PinValue);
//  else
//    Disconnect();
//}

//===============
//EXAMPLE Set a pin state
//===============
//if (CurentlyConnected) {
//  static byte Toggle = 0;
//  Toggle++;
//  if (Toggle > 1)
//    Toggle = 0;
//  if (SetPin(2, Toggle))
//    Serial.println("Pin 2 Set = " + String(Toggle));
//  else
//    Disconnect();
//}

//===============
//EXAMPLE Read data from each slave
//===============
//===============for each slave
//    const static byte AmountOfSlaves = sizeof(SlaveID) / sizeof(String);  //Get amount of slaves
//    for (int i = 0; i < AmountOfSlaves; i++) {
//     //===============Connect
//     if (ConnectTo(SlaveID[i])) {
//        Serial.println("Succesfull connected to " + String(SlaveID[i]));
//        Delay(10);
//        //===============Read
//        String PinValue = ReadPin("B");
//        if (PinValue != "")
//          Serial.println("Pin B = " + PinValue);
//     }
//      //===============disconnect
//      if (Disconnect())
//        Serial.println("Succesfull disconnected");
//      //===============Small delay
//      Delay(150); //Add some delay before sending next command, the master needs to be ready to connect again
//      //===============
//    }
