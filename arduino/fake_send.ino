#include <SoftwareSerial.h>
 
SoftwareSerial mySerial(2, 3); // TX/RX pins

String dataString = "2017,11,19,11,21,50,25,10,1,2,31,32,21,22,201,202\n";
 
void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
}
 
void loop() {
    Serial.println(dataString);   
    mySerial.print(dataString);
    delay(2000);
}
