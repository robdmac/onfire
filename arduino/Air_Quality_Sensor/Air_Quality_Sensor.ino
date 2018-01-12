#include <TinyGPS.h>

#include <DS3234.h>

#include <Time.h>
#include <TimeLib.h>
#include <config.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_ADS1015.h>
#include <SdFat.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <SoftwareSerial.h>
#include <Arduino.h>
#include <TimeLib.h>
#define LENG 31   //0x42 + 31 bytes equal to 32 bytes
#define TIME_HEADER  "T"   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 

DS3234 rtc(10);

int PM01Value=0;          //define PM1.0 value of the air detector module
int PM2_5Value=0;         //define PM2.5 value of the air detector module
int PM10Value=0;         //define PM10 value of the air detector module
const int CS = 10; //
int AlarmPin = 2;

TinyGPS gps;

//Create objects
SdFat sd;
SdFile file;
Adafruit_ADS1115 ads1(0x48);//check with the i2c scanner
Adafruit_ADS1115 ads2(0x49);

unsigned char buf[LENG];
unsigned char wakeup_min;
char newfile[] = "Smokeanalyser.txt";
uint8_t sleep_period = 1;       // the sleep interval in minutes between 2 consecutive alarms
const int cs = 10;
unsigned long time_ms, time_ms_old, interval_ms;
volatile boolean extInterrupt1 = false;    //external interrupt flag1
int counter = 0; 

void set_alarm(void)
{  
    // calculate the minute when the next alarm will be triggered
    //wakeup_min = (t.min / sleep_period + 1) * sleep_period;
    //if (wakeup_min > 59) {
    //    wakeup_min -= 60;
    //}

    // flags define what calendar component to be checked against the current time in order
    // to trigger the alarm - see datasheet
    // A1M1 (seconds) (0 to enable, 1 to disable)
    // A1M2 (minutes) (0 to enable, 1 to disable)
    // A1M3 (hour)    (0 to enable, 1 to disable) 
    // A1M4 (day)     (0 to enable, 1 to disable)
    // DY/DT          (dayofweek == 1/dayofmonth == 0)
    uint8_t flags[4] = { 0, 1, 1, 1};

    // set Alarm1
    //DS3234_set_a2(DS13074_CS_PIN, wakeup_min, 0, 0, flags);

    // activate Alarm1
    //DS3234_set_creg(DS13074_CS_PIN, DS3234_INTCN | DS3234_A2IE);
}

SoftwareSerial BTserial(4,5); // TX / RX
SoftwareSerial PMserial(11,12); // TX / RX
SoftwareSerial GPSserial(42,53); // TX / RX

void setTimeAndDate(){
  // If we ever need to adjust the clocks time and date it is done with this:
  // rtc.setDOW(SUNDAY);        // Set Day-of-Week to SATURDAY
  // rtc.setTime(17, 42, 0);       // Set the time to 12:00:00 (24hr format)
  // rtc.setDate(07, 1, 2018);    // Set the date to January 25th, 
}

void setup(){
  Serial.begin(9600); 
  Serial.println("setup");
  BTserial.begin(9600);
  PMserial.begin(9600);
  GPSserial.begin(9600);
  PMserial.setTimeout(1500);   
  //Serial.setTimeout(1000);    //set the Timeout to 1500ms, longer than the data transmission periodic time of the sensor

  Wire.begin();
  ads1.begin();
  ads2.begin();

  ads1.setGain(GAIN_TWO);    // 2x gain +/- 2.048V
  ads2.setGain(GAIN_TWO);
  
  SPI.beginTransaction(SPISettings(6000000, MSBFIRST, SPI_MODE3));
  rtc.begin();
  delay(1000);
  delay(1000);
  set_alarm();
  delay(1000);

  // pinMode (AlarmPin, INPUT);
  // attachInterrupt(0, alarm, FALLING); // setting the alarm interrupt 

  //while (!sd.begin(CS, SPI_HALF_SPEED)) {
   
  //}
  
  //Serial.println("Card Initialised”); //would print to the Arduino screen if connected
  
  // open the file for write at end like the Native SD library
  //file.open(newfile, O_WRITE | O_CREAT | O_APPEND);
  //file.close();

  delay(1000); 
  Serial.println("done setting up\n");
}
#define BUFF_MAX 256

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
 
void loop(){

  delay (1000);
 
  //  Serial.begin(9600); 
  // gotoSleep();

// Read the time:
  detachInterrupt(0);
  delay(5);
  if (extInterrupt1) extInterrupt1 = false;

  SPI.beginTransaction(SPISettings(6000000, MSBFIRST, SPI_MODE3));
  delay(20);
 
  //DS3234_get(DS13074_CS_PIN, &t);
  set_alarm();
  //DS3234_clear_a2f(DS13074_CS_PIN);
        
  delay(10);
  
  int adsO3_1, adsO3_2, adsNO2_1, adsNO2_2, adsCO_1, adsCO_2, adsCO2_1, adsCO2_2; 
  adsO3_1 = ads1.readADC_SingleEnded(2);
  adsO3_2 = ads1.readADC_SingleEnded(3);
  adsNO2_1 = ads2.readADC_SingleEnded(0);
  adsNO2_2 = ads2.readADC_SingleEnded(1);
  adsCO_1 = ads1.readADC_SingleEnded(0);
  adsCO_2 = ads1.readADC_SingleEnded(1);
  adsCO2_1 = ads2.readADC_SingleEnded(2);
  adsCO2_2 = ads2.readADC_SingleEnded(3);
 
  if(PMserial.find(0x42)){    //start to read when detect 0x42   
    PMserial.readBytes(buf,LENG);
    if(buf[0] == 0x4d){
      PM01Value=transmitPM01(buf); //count PM1.0 value of the air detector module
      PM2_5Value=transmitPM2_5(buf);//count PM2.5 value of the air detector module
      PM10Value=transmitPM10(buf); //count PM10 value of the air detector module 
    }
  }

  Serial.println("a");
  if(GPSserial.available()) {
      Serial.println("b");
    int c = GPSserial.read();
    if(gps.encode(c)) {
        Serial.println("c");
      float latitude, longitude;
      gps.f_get_position(&latitude, &longitude);
      Serial.print("Lat/Long: "); 
      Serial.print(latitude,5); 
      Serial.print(", "); 
      Serial.println(longitude,5);
      int year;
      byte month, day, hour, minute, second, hundredths;
      gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths);
      // Print data and time
      Serial.print("Date: "); Serial.print(month, DEC); Serial.print("/"); 
      Serial.print(day, DEC); Serial.print("/"); Serial.print(year);
      Serial.print("  Time: "); Serial.print(hour, DEC); Serial.print(":"); 
      Serial.print(minute, DEC); Serial.print(":"); Serial.print(second, DEC); 
      Serial.print("."); Serial.println(hundredths, DEC);
      // Here you can print the altitude and course values directly since 
      // there is only one value for the function
      Serial.print("Altitude (meters): "); Serial.println(gps.f_altitude());  
      // Same goes for course
      Serial.print("Course (degrees): "); Serial.println(gps.f_course()); 
      // And same goes for speed
      Serial.print("Speed(kmph): "); Serial.println(gps.f_speed_kmph());
      Serial.println();
      // Here you can print statistics on the sentences.
      unsigned long chars;
      unsigned short sentences, failed_checksum;
      gps.stats(&chars, &sentences, &failed_checksum);
      //Serial.print("Failed Checksums: ");Serial.print(failed_checksum);
      //Serial.println(); Serial.println();
    }
  }

  delay(1000);

    String dataString = "";  //SD Card Storage Write//
        dataString += rtc.getDateStr();
        dataString += ",";
        dataString += rtc.getTimeStr();
        dataString += ",";
        dataString += PM01Value;
        dataString += ",";
        dataString += PM2_5Value;
        dataString += ",";
        dataString += PM10Value;
        dataString += ",";
        dataString += adsCO_1;
        dataString += ",";
        dataString += adsCO_2;
        dataString += ",";
        dataString += adsO3_1;
        dataString += ",";
        dataString += adsO3_2;
        dataString += ",";
        dataString += adsNO2_1;
        dataString += ",";
        dataString += adsNO2_2;
        dataString += ",";
        dataString += adsCO2_1;
        dataString += ",";
        dataString += adsCO2_2;

  Serial.println(dataString);
  BTserial.print("hello\n");
        if (!sd.begin(CS,SPI_HALF_SPEED)) return;
        
        file.open(newfile, O_WRITE | O_APPEND); //Opens the file
        delay(5);
        file.println(dataString); //prints data string to the file
        delay(5);
        file.close(); //closes the file
        delay(5);
        Serial.println(dataString);   
        BTserial.print(dataString); 
        delay(100);    
/*
        BTserial.print(t.year);
        BTserial.print(",");
        BTserial.print(t.mon);
        BTserial.print(",");
        BTserial.print(t.mday);
        BTserial.print(",");
        BTserial.print(t.hour);
        BTserial.print(",");
        BTserial.print(PM01Value);
        BTserial.print(",");
        BTserial.print(PM2_5Value);
        BTserial.print(",");
        BTserial.print(PM10Value);
        BTserial.print(",");
        BTserial.print(adsCO_1);
        BTserial.print(",");
        BTserial.print(adsCO_2);
        BTserial.print(",");
        BTserial.print(adsO3_1);
        BTserial.print(",");
        BTserial.print(adsO3_2);
        BTserial.print(",");
        BTserial.print(adsNO2_1);
        BTserial.print(",");
        BTserial.print(adsNO2_2);
        BTserial.print(",");
        BTserial.print(adsCO2_1);
        BTserial.print(",");
        BTserial.print(adsCO2_2);
*/
        delay(20);
        
  Serial.println("15");
        attachInterrupt(0, alarm, FALLING);
  Serial.println("17");
    
}

char checkValue(unsigned char *thebuf, char leng)
{  
  char receiveflag=0;
  int receiveSum=0;

  for(int i=0; i<(leng-2); i++){
  receiveSum=receiveSum+thebuf[i];
  }
  receiveSum=receiveSum + 0x42;
 
  if(receiveSum == ((thebuf[leng-2]<<8)+thebuf[leng-1]))  //check the serial data 
  {
    receiveSum = 0;
    receiveflag = 1;
  }
  return receiveflag;
}
void gotoSleep(void)
{
  /* Serial.println("1. Going to sleep");
   //Serial.begin(9600); 
   Serial.println("2. Going to sleep");
   byte adcsra = ADCSRA;          //save the ADC Control and Status Register A
   ADCSRA = 0;  //disable the ADC
   sleep_enable();
   power_spi_disable(); 
   set_sleep_mode(SLEEP_MODE_PWR_DOWN);
   cli();
   //sleep_bod_disable();
   sei();
   sleep_cpu();
   /* wake up here */
   /*sleep_disable();
   power_spi_enable(); 
   ADCSRA = adcsra;          */     //restore ADCSRA
}
void alarm()
{
  extInterrupt1 = true;
}
int transmitPM01(unsigned char *thebuf)
{
  int PM01Val;
  PM01Val=((thebuf[3]<<8) + thebuf[4]); //count PM1.0 value of the air detector module
  return PM01Val;
}

//transmit PM Value to PC
int transmitPM2_5(unsigned char *thebuf)
{
  int PM2_5Val;
  PM2_5Val=((thebuf[5]<<8) + thebuf[6]);//count PM2.5 value of the air detector module
  return PM2_5Val;
  }

//transmit PM Value to PC
int transmitPM10(unsigned char *thebuf)
{
  int PM10Val;
  PM10Val=((thebuf[7]<<8) + thebuf[8]); //count PM10 value of the air detector module  
  return PM10Val;
}

