//Arduino Wemos R1 D1 (ESP8266 Boards 2.5.2) & GY-68 BMP180
//SCL=Wemos pin D1 (GPIO5 SCL) SDA=Wemos pin D2 (GPIO4 SDA)
//This plots barometric pressures to the COM Serial Plotter Pa
//BMP180 delivers uncompensated pressure & temperature values
//startPressure(3)--> 0 to 3. 3=hi rez, but slower
//0=low rez, but faster. Routine returns msecs to wait b4 pressure read
#include <SD.h>               //SD card library
#include <SPI.h>              //Serial Peripheral Interface bus lib for COMM, SD com
File dataFile;                //SD card file handle
String SDData;                //Build data to write to SD "disk"
String FName = "0Barom01.txt"; //SD card file name to create/write/read
bool NoPlot = false;          //True = plotting & print extra user data to monitor

#include <Wire.h>                         //Serial library
#include <SFE_BMP180.h>                   //Include BMP180 library
SFE_BMP180 BMP180;                        //Pressure routine
#define ALTITUDE 4.0                      //Device altitude, meters
#define S_prt Serial.print                //Short name for Serial.print
#define S_pln Serial.println              //Short name for Serial.println

char status;                              //Status returned from BMP180
double T, P;                              //Absolute Temp, Pressure
double p0, a;                             //Relative press sealevel, alt
double Tf;                                //Temporary temp F

int j;                                    //For testing

//SD CARD FUNCTIONS =================================
void openSD() {                              //Routine to open SD card
  if (NoPlot){Serial.println(); Serial.println("Open SD card");}    //User message.
  if (!SD.begin(15)) {                       //If not open, print message.  (CS=pin15)
    if (NoPlot){Serial.println("Open SD card failed");}
    return;}
  if (NoPlot){Serial.println("SD Card open");}
}

char openFile(char RW) {                     //Open SD file.  Only 1 open at a time.
  dataFile.close();                          //Ensure file status, before re-opening
  dataFile = SD.open(FName, RW);}            //Open Read at end.  Open at EOF for write/append

String print2File(String tmp1) {             //Print data to SD file
  openFile(FILE_WRITE);                      //Open user SD file for write
  if (dataFile) {                            //If file there & opened --> write
    dataFile.println(tmp1);                  //Print string to file
    dataFile.close();                        //Close file, flush buffer (reliable but slower)
  } else {Serial.println("Error opening file for write");}   //File didn't open
}

void getRecordFile() {                       //Read from SD file
  if (dataFile) {                            //If file is there
    Serial.write(dataFile.read());           //Read datafile, then write to COM.
  } else {Serial.println("Error opening file for read");}    //File didn't open
}
//END SD CARD FUNCTIONS =============================

void(* resetFunc) (void) = 0;             //Reset Arduino function at addr 0

void setup() {
  Serial.begin(9600);                     //Open Com
  delay(500);                             //Allow serial to open
  if (BMP180.begin())                     //Init & if pressure available
    //S_pln("BMP180 opened");               //Tell user success
    int a = 0;
  else {
    S_pln("BMP180 open failed\n");        //Tell user fail
    resetFunc();                          //Restart Arduino  
  }
  openSD();
}
void loop() {
  status = BMP180.startTemperature();     //Start temp reading
  if (status != 0){                       //If start success
    delay(1000);                          //Wait>=4.5 mSec after start b4 get
    status = BMP180.getTemperature(T);    //Start temp reading
    if (status != 0){                     //If temp gotten
      Tf=((T * 1.8) + 32);                //Calc F
      status = BMP180.startPressure(3);   //0-3 0=fast, low rez. 3=slow, hi rez
      if (status != 0){                   //If success
        delay(status);                    //Above function returns needed delay
        status = BMP180.getPressure(P,T); //Get P(press), Pass T(temp)
        if (status != 0){                 //If success
          p0 = BMP180.sealevel(P, ALTITUDE);  //Pressure based on input alt
          a = BMP180.altitude(P, p0);         //Get comp'ed pressure
          S_prt("Bar._Pa:"); S_prt(P,2); S_prt(", ");       //Plot
          S_prt("Bar._Pa_Comp:"); S_prt(p0,2); S_prt(", "); //Plot
          S_pln();
          SDData = String(P) + ',' + String(p0);     //Prepare output string
          print2File(SDData);                        //Write string to SD file
        }
        else S_pln("Temp reading start fail\n");
      }
      else S_pln("Get temperature fail\n");
    }
    else S_pln("Start pressure reading fail\n");
  }
  else S_pln("Get pressure fail\n");
  delay(100);
/*  j++;                                     //Used in testing only.
  if(j > 10){                              //Testing every j writes, dump SD file
    if (NoPlot){Serial.println("File Write Done");}
    openFile(FILE_READ);                   //Testing. Open SD file at start for read. Remove for plotting
    while (dataFile.available()) {         //Testing. Read SD file until EOF
      getRecordFile();}                    //Testing. Get 1 line from SD file
    delay(4000);                           //Testing.Print out data wait x seconds
    j = 0;                                 //Testing only
    dataFile.close();                      //Testing. Close file, flush buffer.
//    if (NoPlot){Serial.println("Printout File Done.  Delete File.");} //Testing only. User info.
//    SD.remove(FName);                        //Testing ONLY. Deletes SD file.    
    }*/

}
