//////////////////////////////////////////////////////////////////
/*________________________FUNCTIONS_____________________________*/
//////////////////////////////////////////////////////////////////





void gsm_preInit(void)
{
    Serial.println("preInit Started !");
    delay(12000);
    sendOnlyCmd("AT\r");
    delay(200);
    sendCmdAndWaitForResp("AT\r", "OK", 5);
    delay(200);
    sendOnlyCmd("ATE0\r");
    delay(200);
    sendOnlyCmd("AT+CIPSHUT\r");
    delay(2000);
    while (Serial2.available())
    {
        Serial2.read();
    }
    Serial.println("preInit Finished !");

}

int gsm_init(void)
{
    char tempChar[15];
    int q = 0;
    GsmAttempts++;
    sendCmdAndWaitForResp("AT+CFUN=1\r", "+CFUN:1\r\n", 150);
    //   delay(200);


    if (0 != sendCmdAndWaitForResp("AT+CPIN?\r", "+CPIN: READY\r\n", 15))
    {
        Serial.println("Initialisation Error 1!!!");
        Serial.println("Restarting GSM 1!!!");
        sendOnlyCmd("AT+CFUN=1,1\r");
        gsm_preInit();
        return -1;
    }
    else {
        Gsm_Init = true;
        Serial.println("GSM initialised!!!");
        GsmAttempts = 0;
        //      return 0;
    }

    sendCmd("AT+CREG?\r");
    delay(2000);

    while (Serial2.available()) {

        tempChar[q] = Serial2.read();

        if (tempChar[q] == '\n' && tempChar[q - 3] == ',') break;
        q++;
        delay(2);
    }


    return 0;

}

int StartGprsConnection()
{
    GprsAttempts++;
    Serial.println("StartGprsConnection");
    sendCmdAndWaitForResp("AT+CSQ\r", "abcd", 15);
    sendCmd("AT+CSQ\r");
    delay(50);
    String responce = "";
    int strength;
    while (Serial2.available())
    {
        strength = Serial2.read();

        if (isDigit(strength))     responce += (char)strength;
        if (',' == (char)strength) break;
        delay(1);
    }
    while (Serial2.available()) Serial2.read();
    strength = responce.toInt();
    Serial.print("strength=");
    Serial.println(strength);
    if (strength > 2)
    {
        //       if(0 != sendCmdAndWaitForResp("AT+CIPSHUT\r","OK\r\n",5))
        //      {
        //        
        //        Serial.println("CIPSHUT nd");
        //  
        //      }
        //      if(0 != sendCmdAndWaitForResp("AT+CGATT?\r","+CGATT: 1\r\n",5))
        //      {
        //        
        //        Serial.println("Gprs Connection Error 1!!!");
        //        Serial.println("Restarting GSM 1!!!");
        //        sendOnlyCmd("AT+CFUN=1,1\r");
        //        gsm_preInit();
        //        return -1;
        //      }
        //      if(0 != sendCmdAndWaitForResp("AT+CSTT=\"CMNET\"\r","OK\r\n",5))
        //      {
        //        
        //        Serial.println("Gprs Connection Error 2!!!");
        //        Serial.println("Restarting GSM 2!!!");
        //        sendOnlyCmd("AT+CFUN=1,1\r");
        //        gsm_preInit();
        //        return -1;
        //      }
        //      if(0 != sendCmdAndWaitForResp("AT+CIICR\r","OK\r\n",5))
        //      {
        //        
        //        Serial.println("Gprs Connection Error 3!!!");
        //        Serial.println("Restarting GSM 3!!!");
        //        sendOnlyCmd("AT+CFUN=1,1\r");
        //        gsm_preInit();
        //        return -1;
        //      }
        //      else
        //      {
        //        sendOnlyCmd("AT+CIFSR\r");
        //        delay(100);
        //      }

        delay(2000);
        while (Serial2.available()) {
            // Serial.write(Serial2.read());
            Serial2.read();

        }

        //Serial.println(udpStr);
        if (0 != sendCmdAndWaitForResp("AT+CIPSTART=\"UDP\",\"210.212.130.075\",\"14001\"\r\n", "CONNECT OK\r\n", 5))
            //  if(0 != sendCmdAndWaitForResp(udpStr,"CONNECT OK\r\n",5))
        {
            Serial.println("Server Connection Error 4!!!");
            Serial.println("Restarting GSM 4!!!");
            sendOnlyCmd("AT+CFUN=1,1\r");
            gsm_preInit();
            return -1;
        }
        else
        {
            Serial.println("Server Connection Successfull");
            //        sendOnlyCmd("AT+CIPSEND\r\n");
            //        sendOnlyCmd(IMEI);
            //        sendEndMark();
            //        delay(1000);
            //        sendOnlyCmd("AT+CIPSHUT\r\n");
            GprsState = 0;
            GprsAttempts = 0;
            return 0;
        }


    }
    else {
        Serial.println("Signal Strength Insufficient");
    }






}

int sendCmdAndWaitForResp(const char* cmd, const char* resp, unsigned timeout)
{

    sendCmd(cmd);
    return waitForResp(resp, timeout);
}


int waitForResp(const char* resp, unsigned int timeout)
{

    int len = strlen(resp);
    int sum = 0;
    unsigned long timerStart, timerEnd;
    timerStart = millis();

    while (1) {
        if (Serial2.available()) {
            char c = Serial2.read();
            Serial.print(c);
            delay(1);
            sum = (c == resp[sum]) ? sum + 1 : 0;
            if (sum == len)break;
        }
        timerEnd = millis();
        if (timerEnd - timerStart > 1000 * timeout) {
            return -1;
        }
    }

    while (Serial2.available()) {
        Serial2.read();
        delay(1);
    }

    return 0;
}

int waitForResp1(const char* resp, unsigned int timeout)
{

    int len = strlen(resp);
    int sum = 0;
    unsigned long timerStart, timerEnd;
    timerStart = millis();

    while (1) {
        if (Serial2.available()) {
            char c = Serial2.read();
            //            Serial.print(c);
            delay(1);
            sum = (c == resp[sum]) ? sum + 1 : 0;
            if (sum == len)break;
        }
        timerEnd = millis();
        if (timerEnd - timerStart > 1000 * timeout) {
            return -1;
        }
    }

    while (Serial2.available()) {
        Serial2.read();
        delay(1);
    }

    return 0;
}

int CheckGsmStatus(void)
{
    int ret = sendCmdAndWaitForResp("AT\r", "OK\r\n", 5);
    return ret;
}

void sendCmd(const char* cmd)
{
    //   Serial2.listen();
    Serial2.write(cmd);
}

void sendOnlyCmd(const char* cmd)
{
    //    Serial2.listen();
    Serial2.write(cmd);
    delay(100);
    while (Serial2.available()) {
        Serial2.read();
    }
}

void sendEndMark(void)
{
    sendCmd((char)26);
}

void send_salinity_comp(unsigned int channel){
  Wire.beginTransmission(channel_ids[channel]);     // call the circuit by its ID number.
  String cmd="";
  cmd=String("S,")+String(salinity)+String(",ppt");
  const char* cmd_pointer=cmd.c_str();
  Wire.write(cmd_pointer);                          // request a reading by sending 'rt'
  Wire.endTransmission();                         // end the I2C data transmission.
  delay(300);
}


void readSensorWithTempComp(unsigned int channel)
{
  Wire.beginTransmission(channel_ids[channel]);     // call the circuit by its ID number.
  String cmd="";
  cmd=String("rt,")+String(temp,3);
  const char* cmd_pointer=cmd.c_str();
  Wire.write(cmd_pointer);                          // request a reading by sending 'rt'
  Wire.endTransmission();                         // end the I2C data transmission.
  delay(900);
  sensor_bytes_received = 0;                        // reset data counter
  memset(sensordata, 0, sizeof(sensordata));        // clear sensordata array;

  Wire.requestFrom(channel_ids[channel], 48, 1);    // call the circuit and request 48 bytes (this is more then we need).
  code = Wire.read();
    //    Serial.print("code=");
     //   Serial.println(code);
    while (Wire.available()) {          // are there bytes to receive?
        in_char = Wire.read();            // receive a byte.

        if (in_char == 0) {               // null character indicates end of command
            Wire.endTransmission();         // end the I2C data transmission.
            break;                          // exit the while loop, we're done here
        }
        else {
            sensordata[sensor_bytes_received] = in_char;      // append this byte to the sensor data array.
           // Serial.print(sensordata[sensor_bytes_received]);
            sensor_bytes_received++;
            sat_com[sat_len] = in_char;
            //  Serial.print(in_char);
            sat_len++;
        }
    }
    
 //   Serial.println(sensordata);

    
}




void readSensor(unsigned int channel)
{
    Wire.beginTransmission(channel_ids[channel]);     // call the circuit by its ID number.
    Wire.write('r');                          // request a reading by sending 'r'
    Wire.endTransmission();                         // end the I2C data transmission.

    delay(900);  // AS circuits need a 1 second before the reading is ready

    sensor_bytes_received = 0;                        // reset data counter
    memset(sensordata, 0, sizeof(sensordata));        // clear sensordata array;

    Wire.requestFrom(channel_ids[channel], 48, 1);    // call the circuit and request 48 bytes (this is more then we need).
    code = Wire.read();
    //    Serial.print("code=");
     //   Serial.println(code);
    while (Wire.available()) {          // are there bytes to receive?
        in_char = Wire.read();            // receive a byte.

        if (in_char == 0) {               // null character indicates end of command
            Wire.endTransmission();         // end the I2C data transmission.
            break;                          // exit the while loop, we're done here
        }
        else {
            sensordata[sensor_bytes_received] = in_char;      // append this byte to the sensor data array.
           // Serial.print(sensordata[sensor_bytes_received]);
            sensor_bytes_received++;
            sat_com[sat_len] = in_char;
            //  Serial.print(in_char);
            sat_len++;
        }
    }
 //   Serial.println(sensordata);
}
