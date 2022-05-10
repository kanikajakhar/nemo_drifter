// WhiteBox Labs -- Tentacle Shield -- I2C example
// www.whiteboxes.ch
//
// How to retrieve continuous sensr readings from op to 8 Atlas Scientific devices on the I2C bus
// and send the readings to a host computer via serial.
//
// This code is intended to work on all 5V tolerant Arduinos. If using the Arduino Yun, connect
// to it's usb serial port. If you want to work with the Yun wirelessly, check out the respective
// Yun version of this example.
//
// USAGE:
//---------------------------------------------------------------------------------------------
// - Set all your EZO circuits to I2C before using this sketch.
//    - You can use the "tentacle-steup.ino" sketch to do so)
//    - Make sure each circuit has a unique I2C ID set
// - Adjust the variables below to resemble your setup: TOTAL_CIRCUITS, channel_ids, channel_names
// - Set host serial terminal to 9600 baud
//
//---------------------------------------------------------------------------------------------
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------------------------------

#include <Wire.h>                     // enable I2C.
//#include <TimerOne.h>
#include <SD.h>
#include <SPI.h>
#include <EEPROM.h>
#include <String.h>

char sensordata[30];                  // A 30 byte character array to hold incoming data from the sensors
byte sensor_bytes_received = 0;       // We need to know how many characters bytes have been received

byte code = 0;                        // used to hold the I2C response code.
byte in_char = 0;                     // used as a 1 byte buffer to store in bound bytes from the I2C Circuit.
String str;
//String strOne;
bool strComplete = false;
char inByte;
char MSG[6];
char DATA_RMC[100];
char DATA_GGA[100];
char sat_com[150];
char sat_com1[150];
int sat_len;
float temp;
float salinity;
char sal[5];
char udpStr[60];
Sd2Card card;
char IpAdd[16] = "210.212.130.075";

char port[6] = "14001";
unsigned char GprsState = 20;
unsigned char GprsAttempts = 0;
bool Gsm_Init = false;
unsigned char GsmAttempts = 0;
unsigned char count = 0;
const int DEFAULT_TIMEOUT = 5;
#define TOTAL_CIRCUITS 4              // <-- CHANGE THIS | set how many I2C circuits are attached to the Tentacle shield(s): 1-8

int analoginput=0;
String analogamount;
float percentage=0;
float voltage=0;
const int chipSelect = 8;

//int channel_ids[] = { 97, 102, 99, 100 };// <-- CHANGE THIS.
int channel_ids[] = {102, 99, 100, 97 };
// A list of I2C ids that you set your circuits to.
// This array should have 1-8 elements (1-8 circuits connected)


//char* channel_names[] = { "DO", "RTD", "PH", "EC" }; // <-- CHANGE THIS.
char* channel_names[] = {"RTD", "PH", "EC", "DO" };
// A list of channel names (must be the same order as in channel_ids[]) 
// it's used to give a name to each sensor ID. This array should have 1-8 elements (1-8 circuits connected).
// {"PH Tank 1", "PH Tank 2", "EC Tank 1", "EC Tank2"}, or {"PH"}
File myFile;
int value;
char filename[8];
String filestring;
//filename[15]=
//ph needs temp comp
//ec needs temp comp
//do needs temp comp and salinity compensation

void setup() {                 // startup function

  analogReference(INTERNAL2V56);
    Serial.begin(9600);              // Set the hardware serial port.
    Serial1.begin(9600);
    Serial2.begin(9600);
    Wire.begin();         // enable I2C port.
    Serial.print("start");

    sat_com[0] = '$';
    sat_com[1] = 'N';
    sat_com[2] = 'M';
    sat_com[3] = '1';
    sat_com[4] = ',';


    gsm_preInit();
    pinMode(chipSelect, OUTPUT);

    if (!SD.begin(chipSelect)) {
        Serial.println(F("Card failed, or not present"));
    }
    else {
        Serial.println(F("card initialized."));
      //  myFile = SD.open("test.txt", FILE_WRITE);
        //if (myFile) {
          //  Serial.print("Writing to SD Card");
            //myFile.println("testing 1 2 3 ");
     //       myFile.close();
       // }
    }

    while (0 != gsm_init()) {
        if (GsmAttempts >= 2) {
            GprsState = 20;
            //    mySerial.println(F("gsm_init Failed !!!"));
            break;
        }
        delay(1000);
    }

    snprintf(udpStr, sizeof(udpStr), "AT+CIPSTART=\"UDP\",\"%s\",\"%s\"\r", IpAdd, port);


    if (Gsm_Init) {
        //         Serial.println(F("GprsConnection Initialization Started !!!"));
        while (0 != StartGprsConnection()) {
            if (GprsAttempts > 2) {
                GprsState = 20;
                //        Serial.println(F("Connecting UDP Server Failed !"));
                break;
            }
        }
    }
    
    value=EEPROM.read(0);
    Serial.println(value);
    if(value<255){
      value++;
    }
    else{
      value=0;
    }
    Serial.println("new value");
    Serial.println(value);
    EEPROM.write(0,value);
    
    sprintf(filename,"d%03d.txt",value);
    filestring=String(filename);
    //filename="datalog" + String(value) + ".txt";
    Serial.println(filestring);
}



void loop() {

    if (strComplete == true) {
            sat_com[63] = ',';
        sat_len = 64;

        //strOne = "";

        Serial.println("strcplt");
        int channel=0;

        readSensor(channel);
        temp=atof(sensordata);
        //strOne = //strOne + ',' + sensordata;

        sat_com[sat_len] = ',';
        sat_len++;

        for (int channel = 1; channel < TOTAL_CIRCUITS; channel++) {
    //      Serial.print(channel_names[channel]);
     //     Serial.print(':');

          if(channel==3){
            send_salinity_comp(channel);
          }
          
            readSensorWithTempComp(channel);
            //strOne = //strOne + ',' + sensordata;

            if(channel==2){
              memset(sal, 0, sizeof(sal)); 
              int comma_count=0;
              for(int x=0;x<sizeof(sensordata);x++){
                if(sensordata[x]==','){
                  comma_count++;
                  
                }
                if(comma_count==2){
                  x++;
              //    Serial.print("salinity:");
                  for(int fa=0;fa<5;fa++){
                    if(sensordata[x]==','){
                      break;
                    }
                    sal[fa]=sensordata[x];
              //      Serial.print(sensordata[x]);
                    
                    x++;
                  }
                  break;
                }
              }

              salinity=atof(sal);
             // Serial.print("salinity:");
           //   Serial.println(" ");
          //    Serial.print("salinityas ");
           //   Serial.println(salinity);
            }

            

            if (channel < TOTAL_CIRCUITS) {
                sat_com[sat_len] = ',';
                sat_len++;
            }
        }


        analogamount=String(analogRead(analoginput));
        //perecentage=(analogamount/1024)*100;
        voltage=((analogamount.toInt())*2.56)/1024;
        Serial.print("voltage: "); 
        Serial.println(voltage); 
        Serial.print("analogamount: "); 
        Serial.println(analogamount); 
        for(int fet=0;fet< analogamount.length();fet++){
          sat_com[sat_len] =analogamount[fet];
          sat_len++;
        }
        
        sat_com[sat_len] = '*';
        sat_len++;

        sat_com[sat_len] = '\0';
        Serial.println(&sat_com[0]);

        myFile = SD.open(filestring, FILE_WRITE);
        if (myFile) {
            myFile.print(&sat_com[0]);
            myFile.close();
            Serial.println("sd card write success");
        }
        else{
          Serial.println("sd card write fail");
        }
//AP-BTZ5L AMARON PRO RIDER BETA SERIES

        if (GprsState == 0)
        {
            if (0 == sendCmdAndWaitForResp("AT+CIPSEND\r", "> ", DEFAULT_TIMEOUT))
            {
                //                mySerial.println("Sending Data To UDP Server");

                Serial2.print(&sat_com[0]);
                //   Serial2.println(&sat_com1[0]);


                Serial2.write(26);

                if (0 == waitForResp1("SEND OK", DEFAULT_TIMEOUT))
                {
                    //                  mySerial.println(F("Sending Data To UDP Server Successfull"));

                }

                else
                {

                    //                  GprsState = false;
                           //           mySerial.println(F("Sending Data To UDP Server Failed"));
                    sendOnlyCmd("AT+CIPSHUT\r\n");
                    delay(5000);
                    while (Serial2.available())
                    {
                        Serial2.read();
                    }
                    if (StartGprsConnection() == -1)GprsState = 20;
                }
            }
            else
            {                                     //If data sending fails, then all GPRS connections are closed and restablished

//                  GprsState = false;
          //        mySerial.println(F("Sending Data To UDP Server Failed"));  
                sendOnlyCmd("AT+CIPSHUT\r\n");
                delay(5000);
                while (Serial2.available())
                {
                    Serial2.read();
                }
                if (StartGprsConnection() == -1)GprsState = 20;
            }
        }
        else {

            if (GprsState > 0)GprsState--;
            if (GprsState < 0)GprsState = 0;
        } //gprs loop


        strComplete = false;


    }


}

/*PH:0.000
EC:0.00,0,0.00,1.000
DO:117.88
RTD:-1023.000
PH:0.000
EC:0.00,0,0.00,1.000
DO:117.88
*/

void serialEvent1() {
    if (strComplete == false) {
        if (Serial1.available()) {
            int i;
            inByte = Serial1.read();
            if (inByte == '$') {
                MSG[0] = inByte;
                str = "";
                for (int k = 1; k < 6; k++)
                {
                    while (Serial1.available() <= 0) {}
                    MSG[k] = Serial1.read();

                }
                for (int k = 3; k < 6; k++)
                {
                    str += MSG[k];

                }
                if (str == "RMC")
                {
                    //  Serial.print(str);
                    int k;
                    for (k = 0; k < 6; k++) {
                        DATA_RMC[k] = MSG[k];
                    }
                    for (k = 6; k < 65; k++)
                    {
                        while (Serial1.available() <= 0) {}
                        inByte = Serial1.read();

                        if (inByte == '\n')
                        {
                            DATA_RMC[k] = inByte;
                            k++;
                            break;
                        }
                        DATA_RMC[k] = inByte;
                    }
                    Serial.println(&DATA_RMC[0]);

                    Serial.println(&DATA_RMC[0]);
                    DATA_RMC[k + 1] = '\0';

                    for (int x = 0; x < 58; x++) {
                        sat_com[x + 5] = DATA_RMC[x + 7];           //time-lat-lon-speed-direction-date
                    }
                    //     sat_com[63] = '$';
                    //     sat_com[64] = '\0';
                     //    Serial.println(&sat_com[0]);


                    strComplete = true;
                }

            }
            //  str=Serial1.readStringUntil('\n');
            //  strComplete=true;
        }
    }
}
