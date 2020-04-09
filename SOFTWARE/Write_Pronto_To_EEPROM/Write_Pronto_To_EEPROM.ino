
#include <EEPROM.h>
static int incomingByte = 0;
static int prevByte=0;
static int cmd=5;
static bool rec_but = false;
static uint16_t array[80];
static uint16_t ars = 0;
int buttonPin2 = 5;  
int buttonPin1 = 4;  
int buttonState1 = 0;        
int buttonState2 = 0;
int trigger_state = 1;



void setup() {

pinMode(buttonPin1, INPUT_PULLUP);  
pinMode(buttonPin2, INPUT_PULLUP);
for(ars=0;ars<EEPROM.length();ars++){
  EEPROM.write(ars,0);
}

Serial.begin(115200); // opens serial port, sets data rate to 9600 bps

}

void loop() {


uint16_t j = 0;
uint16_t count = 1;



buttonState1 = digitalRead(buttonPin1);
buttonState2 = digitalRead(buttonPin2);

if (trigger_state == LOW && buttonState2 == HIGH){
  Serial.println("OK");
}
trigger_state = buttonState2;

if(buttonState2==LOW){
  char buf[50];
  sprintf(buf, "Please enter valid Pronto HEX code for command %d", cmd);
  Serial.println(buf);
  
  if(cmd<0){
    cmd=5;
  }
}

if(buttonState1==LOW ){

if(rec_but){
 int address=0;
  char buf[50];
  sprintf(buf, "CMD is %d", cmd);
    Serial.println(buf);
  Serial.println("Writing Pronto Code to EEPROM...");
 if(cmd==5){
  address = 0;
 }
 if(cmd==4){
  address = 160;
 }
 if(cmd==3){
  address=320;
 }
 if(cmd==2){
  address=480;
 }
if(cmd==1){
  address=640;
 }

 if(cmd==0){
  address=800;
 }
 
 EEPROM.put( address, array );
 cmd--;
 Serial.println("OK");
 Serial.println("Press and Hold again the recording button to add new pronto HEX Code!");
 
}
 rec_but = false;
}

while (buttonState2==LOW){

buttonState2 = digitalRead(buttonPin2);
trigger_state =LOW;

if (Serial.available() > 0) {

incomingByte = Serial.read(); // read the incoming byte:



if(count % 2 == 0){
 array[j++] =  uint16_t((prevByte << 8) | incomingByte);


}else{

  prevByte = incomingByte;
}
count++;
}
rec_but = true;
 



}

delay(1000);
//Serial.println(array[0]);
//Serial.println(array[1]);

}
