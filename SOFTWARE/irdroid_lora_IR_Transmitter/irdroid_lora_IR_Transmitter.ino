/*******************************************************************************
   Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman

   Permission is hereby granted, free of charge, to anyone
   obtaining a copy of this document and accompanying files,
   to do whatever they want with them without any restriction,
   including, but not limited to, copying, modification and redistribution.
   NO WARRANTY OF ANY KIND IS PROVIDED.

   This example sends a valid LoRaWAN packet with payload "Hello,
   world!", using frequency and encryption settings matching those of
   the The Things Network.

   This uses OTAA (Over-the-air activation), where where a DevEUI and
   application key is configured, which are used in an over-the-air
   activation procedure where a DevAddr and session keys are
   assigned/generated for use with all further communication.

   Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in
   g1, 0.1% in g2), but not the TTN fair usage policy (which is probably
   violated by this sketch when left running for longer)!

   To use this sketch, first register your application and device with
   the things network, to set or generate an AppEUI, DevEUI and AppKey.
   Multiple devices can use the same AppEUI, but each device has its own
   DevEUI and AppKey.

   Do not forget to define the radio type correctly in config.h.

 *******************************************************************************/

#include <lmic.h>
#include <hal/hal.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <avr/io.h>
#include <EEPROM.h>
static int incomingByte = 0;
static int prevByte=0;
static int cmd=5;
static bool rec_but = false;
static bool trigger = false;
int buttonPin2 = 5;  
int buttonPin1 = 4;  
int buttonState1 = 0;        
int buttonState2 = 0;
int trigger_state = 1;
#define IR_PORT PORTB
#define IR_PIN PINB
#define IR_DDR DDRB
#define IR_BV _BV(1)
#define IR_OCR OCR1A
#define IR_TCCRnA TCCR1A
#define IR_TCCRnB TCCR1B
#define IR_TCNTn TCNT1
#define IR_TIFRn TIFR1
#define IR_TIMSKn TIMSK1
#define IR_TOIEn TOIE1
#define IR_ICRn ICR1
#define IR_OCRn OCR1A
#define IR_COMn0 COM1A0
#define IR_COMn1 COM1A1
#define PRONTO_IR_SOURCE 0 // Pronto code byte 0
#define PRONTO_FREQ_CODE 1 // Pronto code byte 1
#define PRONTO_SEQUENCE1_LENGTH 2 // Pronto code byte 2
#define PRONTO_SEQUENCE2_LENGTH 3 // Pronto code byte 3
#define PRONTO_CODE_START 4 // Pronto code byte 4
static bool firstrun = false;
static const uint16_t *ir_code = NULL;
static uint16_t ir_cycle_count = 0;
static uint32_t ir_total_cycle_count = 0;
static uint8_t ir_seq_index = 0;
static uint8_t ir_led_state = 0;
static boolean rec = false;
//static char *message = "SEND 0000 006d 0022 0003 00a9 00a8 0015 003f 0015 003f 0015 003f 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 003f 0015 003f 0015 003f 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 003f 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0040 0015 0015 0015 003f 0015 003f 0015 003f 0015 003f 0015 003f 0015 003f 0015 0702 00a9 00a8 0015 0015 0015 0e6e";

static uint16_t array[80];

void ir_on()
{
  IR_TCCRnA |= (1<<IR_COMn1) + (1<<IR_COMn0);
  ir_led_state = 1;
}

void ir_off()
{
  IR_TCCRnA &= ((~(1<<IR_COMn1)) & (~(1<<IR_COMn0)) );
  ir_led_state = 0;
}

void ir_toggle()
{
  if (ir_led_state)
    ir_off();
  else
    ir_on();
}

void ir_start(uint16_t *code)
{
  ir_code = code;
  IR_PORT &= ~IR_BV; // Turn output off
  IR_DDR |= IR_BV; // Set it as output
  IR_TCCRnA = 0x00; // Reset the pwm
  IR_TCCRnB = 0x00;
  //printf_P(PSTR("FREQ CODE: %hd\r\n"), code[PRONTO_FREQ_CODE]);
  uint16_t top = ( (F_CPU/1000000.0) * code[PRONTO_FREQ_CODE] * 0.241246 ) - 1;
  //printf_P(PSTR("top: %hu\n\r"), top);
  IR_ICRn = top;
  IR_OCRn = top >> 1;
  IR_TCCRnA = (1<<WGM11);
  IR_TCCRnB = (1<<WGM13) | (1<<WGM12);
  IR_TCNTn = 0x0000;
  IR_TIFRn = 0x00;
  IR_TIMSKn = 1 << IR_TOIEn;
  ir_seq_index = PRONTO_CODE_START;
  ir_cycle_count = 0;
  ir_on();
  IR_TCCRnB |= (1<<CS10);
}

#define TOTAL_CYCLES 80000 // Turns off after this number of
// cycles. About 2 seconds
// FIXME: Turn off after having sent
ISR(TIMER1_OVF_vect) {
  uint16_t sequenceIndexEnd;
  uint16_t repeatSequenceIndexStart;
  ir_total_cycle_count++;
  ir_cycle_count++;

  if (ir_cycle_count== ir_code[ir_seq_index]) {
    ir_toggle();
    ir_cycle_count = 0;
    ir_seq_index++;
    sequenceIndexEnd = PRONTO_CODE_START +
      (ir_code[PRONTO_SEQUENCE1_LENGTH]<<1) +
      (ir_code[PRONTO_SEQUENCE2_LENGTH]<<1);

    repeatSequenceIndexStart = PRONTO_CODE_START +
      (ir_code[PRONTO_SEQUENCE1_LENGTH]<<1);

    if (ir_seq_index >= sequenceIndexEnd ) {
      ir_seq_index = repeatSequenceIndexStart;

      if(ir_total_cycle_count>TOTAL_CYCLES) {
        ir_off();
        TCCR1B &= ~(1<<CS10);
      }
    }
  
  }
}
// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
static const u1_t PROGMEM APPEUI[8] = { 0x46, 0xBC, 0x02, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
void os_getArtEui (u1_t* buf) {
  memcpy_P(buf, APPEUI, 8);
}

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8] = { 0x00, 0x00, 0xA0, 0x70, 0x00, 0x0B, 0x00, 0x0E };
void os_getDevEui (u1_t* buf) {
  memcpy_P(buf, DEVEUI, 8);
}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
// The key shown here is the semtech default key.
static const u1_t PROGMEM APPKEY[16] = { 0x02, 0xD0, 0x60, 0x0B, 0xE0, 0x23, 0x04, 0x00, 0x0B, 0x0F, 0xF0, 0x4E, 0x0B, 0x10, 0x4D, 0x05 };
void os_getDevKey (u1_t* buf) {
  memcpy_P(buf, APPKEY, 16);
}

static uint8_t mydata[] = "N";
static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 40;

// Pin mapping
const lmic_pinmap lmic_pins = {
  .nss = 10,
  .rxtx = LMIC_UNUSED_PIN, // Not connected on RFM92/RFM95
  .rst = 4,  // Needed on RFM92/RFM95
  .dio = {2, 6, 7},
};
//void tx(){
//    
//       uint16_t j = 0;
//
//     if ( !strncmp(message, "SEND", 4) )
//      {
//       
//        char* p = message + 4;
//
//        while ( (p = strchr(p, ' ')) != NULL ){
//          array[j++] = strtol(p, &p, 16);
//        }
//      //  ir_start(array);
//       
//      }
//      
//      
//}
void onEvent (ev_t ev) {


  Serial.print(os_getTime());
  Serial.print(": ");
  switch (ev) {
    case EV_SCAN_TIMEOUT:
      Serial.println(F("EV_SCAN_TIMEOUT"));
      break;
    case EV_BEACON_FOUND:
      Serial.println(F("EV_BEACON_FOUND"));
      break;
    case EV_BEACON_MISSED:
      Serial.println(F("EV_BEACON_MISSED"));
      break;
    case EV_BEACON_TRACKED:
      Serial.println(F("EV_BEACON_TRACKED"));
      break;
    case EV_JOINING:
      Serial.println(F("EV_JOINING"));
      break;
    case EV_JOINED:
      Serial.println(F("EV_JOINED"));
       firstrun=true; 
      

      // Disable link check validation (automatically enabled
      // during join, but not supported by TTN at this time).
      LMIC_setLinkCheckMode(0);
      break;
    case EV_RFU1:
      Serial.println(F("EV_RFU1"));
      break;
    case EV_JOIN_FAILED:
      Serial.println(F("EV_JOIN_FAILED"));
      break;
    case EV_REJOIN_FAILED:
      Serial.println(F("EV_REJOIN_FAILED"));
      break;
      break;
    case EV_TXCOMPLETE:
      Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
    
     // if (LMIC.txrxFlags & TXRX_ACK)
     //   Serial.println(F("Received ack"));
      if (LMIC.dataLen) {
        Serial.println(F("Received "));
        Serial.println(LMIC.dataLen);
        Serial.println(F(" bytes of payload"));
     //   IR_PORT &= ~IR_BV;
   trigger = true;
    
   //   rec=true;
      }
      // Schedule next transmission
      os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
      break;
    case EV_LOST_TSYNC:
      Serial.println(F("EV_LOST_TSYNC"));
      break;
    case EV_RESET:
      Serial.println(F("EV_RESET"));
      break;
    case EV_RXCOMPLETE:
      // data received in ping slot
      Serial.println(F("EV_RXCOMPLETE"));
      break;
    case EV_LINK_DEAD:
      Serial.println(F("EV_LINK_DEAD"));
      break;
    case EV_LINK_ALIVE:
      Serial.println(F("EV_LINK_ALIVE"));
      break;
    default:
      Serial.println(F("Unknown event"));
      break;
  }
}

void do_send(osjob_t* j) {
  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println(F("OP_TXRXPEND, not sending"));
  } else {
    // Prepare upstream data transmission at the next possible time.
    LMIC_setTxData2(1, mydata, sizeof(mydata) - 1, 0);
    Serial.println(F("Packet queued"));
    Serial.println(LMIC.freq);
  }
  // Next TX is scheduled after TX_COMPLETE event.
}

void setup() {
   EEPROM.get( 0, array );
//  pinMode(buttonPin1, INPUT_PULLUP);  
//  pin//Mode(buttonPin2, INPUT_PULLUP);
  Serial.begin(115200);
  Serial.println(F("Starting"));
 
 
  //Serial.println(F("YangYH test 1"));
#ifdef VCC_ENABLE
  // For Pinoccio Scout boards
  pinMode(VCC_ENABLE, OUTPUT);
  digitalWrite(VCC_ENABLE, HIGH);
  delay(1000);
#endif

  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();
  LMIC.txChnl = 0;
  
  LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);
 
  printotaainformation();
 // Define the single channel and data rate (SF) to use
int channel = 0;
int dr = DR_SF7;

#define CHANNEL  0

for (uint8_t i = 0; i < 9; i++) {
  if (i != CHANNEL) {
    LMIC_disableChannel(i);
  }
}
 //LMIC_setAdrMode(0); 
   LMIC_setLinkCheckMode(0);
// Set data rate (SF) and transmit power for uplink
LMIC_setDrTxpow(dr, 20);
  // Start job (sending automatically starts OTAA too)
 //  tx();
  do_send(&sendjob);
}
void printotaainformation(void)
{
  unsigned char i;
  unsigned char chartemp;
  unsigned char messagelength;

  Serial.println(F("OTAA mode to join network"));
  Serial.println(F("In ThingPark China Platform, please use following information to create this device !"));
  Serial.print("DevEui: ");
  for (i = 0; i <= 7; i++)
  {
    chartemp = pgm_read_word_near(DEVEUI+7-i);
    covertandprint((chartemp >> 4) & 0xf);
    covertandprint(chartemp & 0xf);    
  }
  Serial.println("");
  Serial.print("AppEui: ");
  for (i = 0; i <=7; i++)
  {
    chartemp = pgm_read_word_near(APPEUI+7-i);
    covertandprint((chartemp >> 4) & 0xf);
    covertandprint(chartemp & 0xf);    
  }

  Serial.println("");
  Serial.print("AppKey: ");
  //memcpy_P(buftemp, APPKEY, 16);
  for (i = 0; i <= 15; i++)
  {
    chartemp = pgm_read_word_near(APPKEY+i);
    //Serial.print(buftemp[i],HEX);  // 使用这个语句，不打印0字符
    covertandprint((chartemp >> 4) & 0xf);
    covertandprint(chartemp & 0xf);
  }
  Serial.println("");
  
  Serial.println("In this SW will send following information to network(uplink), you can see them in ThingPark Platform Wireless Logger window");
 // messagelength = strlen(mydata);
  //for (i = 0; i <= messagelength-1; i++)
  //{
   // Serial.print(char(mydata[i]));
  //}
  Serial.print((char*)mydata);
  Serial.println("");
  Serial.println(""); // add one new line
}

void covertandprint(unsigned char value)
{
  switch (value)
  {
    case 0  : Serial.print("0"); break;
    case 1  : Serial.print("1"); break;
    case 2  : Serial.print("2"); break;
    case 3  : Serial.print("3"); break;
    case 4  : Serial.print("4"); break;
    case 5  : Serial.print("5"); break;
    case 6  : Serial.print("6"); break;
    case 7  : Serial.print("7"); break;
    case 8  : Serial.print("8"); break;
    case 9  : Serial.print("9"); break;
    case 10  : Serial.print("A"); break;
    case 11  : Serial.print("B"); break;
    case 12  : Serial.print("C"); break;
    case 13  : Serial.print("D"); break;
    case 14  : Serial.print("E"); break;
    case 15 :  Serial.print("F"); break;
    default :
      Serial.print("?");   break;
  }
}
const uint16_t inputLength = 256;

void loop() {

  
  os_runloop_once();
  
  if(firstrun){
     
      if(LMIC.frame[LMIC.dataBeg + 0] == 0xA1 && trigger){
      EEPROM.get( 0, array );
      rec = true;
      trigger=false;
      }
      if(LMIC.frame[LMIC.dataBeg + 0] == 0xA2 && trigger){
      EEPROM.get( 160, array );
      rec = true;
       trigger=false;
      }
       if(LMIC.frame[LMIC.dataBeg + 0] == 0xA3 && trigger){
      EEPROM.get( 320, array );
      rec = true;
       trigger=false;
      }
        if(LMIC.frame[LMIC.dataBeg + 0] == 0xA4 && trigger){
      EEPROM.get( 480, array );
      rec = true;
       trigger=false;
      }
      if(LMIC.frame[LMIC.dataBeg + 0] == 0xA5 && trigger){
      EEPROM.get( 640, array );
      rec = true;
       trigger=false;
      }
       if(LMIC.frame[LMIC.dataBeg + 0] == 0xA6 && trigger){
      EEPROM.get( 800, array );
      rec = true;
       trigger=false;
      }
     }




   
if(rec){

 ir_start(array);
  rec = false;
 // delay(1000);
}
 

   
 
   
}
