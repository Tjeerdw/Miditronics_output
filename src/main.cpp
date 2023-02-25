#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "config.h"
#include "outputs.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMono9pt7b.h>
#include <MIDI.h>
#include <ArduinoNvs.h>

//config midi instance on serial 2
#ifdef SERIALMIDI
  MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);
#else  
  MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);
#endif

//config (temp) variables for midi implementation, some are waiting on menu implementation and eeprom storage

//Persistent variables through EEPROM
uint8_t MidiChannel=1;          
bool isRegisterModule = false;  // TODO: Make Enum
bool isOutputModule = false;    // TODO: Make Enum
uint8_t registerOffSet = 0;     // registeroffset in geval van extra registermodule
uint8_t startNoot = 23;         // midi-nootnummer waarop deze module moet starten (23 = C1, 36 = C2, 49 = C3) TODO: make array with notes

const int controlChangeAan = 80;//control change waarde aan
const int controlChangeUit = 81;//control change waarde uit
uint8_t totaalModuleKanalen = 0;   //definieert het aantal kanalen dat deze module kan aansturen (32/64)
uint16_t actualInputs[4] = {0,0,0,0};
uint16_t previousInputs[4] = {0,0,0,0};
#define eindNoot  startNoot+totaalModuleKanalen //midi-nootnummer waar deze module stopt met reageren


TwoWire display_I2C =  TwoWire(0);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &display_I2C, OLED_RESET);

void loadNVSSettings(){
  MidiChannel = NVS.getInt("channel");
  isRegisterModule = NVS.getInt("regmodule");
  registerOffSet = NVS.getInt("regoffset");
  startNoot = NVS.getInt("startnoot");
}

void saveNVSSettingsReset(){
  NVS.setInt("channel",MidiChannel);
  NVS.setInt("regmodule",isRegisterModule);
  NVS.setInt("regoffset",registerOffSet);
  NVS.setInt("startnoot", startNoot);
  ESP.restart();
}

//note-On message afhandelen
void handleNoteOn(byte incomingChannel, byte pitch, byte velocity){
  if (!isRegisterModule) {  
    velocity = 127; //ter ere van Hendrikus
    if ((pitch>=startNoot) && (pitch<=eindNoot)) {
      pitch = (pitch-(startNoot-1)); //converteert noot naar het juiste outputnummer        
      setOutput(pitch,HIGH); //schakel noot in
    }
  }
}

//note-Off message afhandelen
void handleNoteOff(byte incomingChannel, byte pitch, byte velocity){
  if (!isRegisterModule) {
    velocity = 127; //ter ere van Hendrikus
    if ((pitch>=startNoot) && (pitch<=eindNoot)) {
      pitch = (pitch-(startNoot-1)); //converteert noot naar het juiste outputnummer
      setOutput(pitch,LOW); //schakel noot uit
    } 
  }
}

//Control-Change message afhandelen
void handleControlChange(byte incomingChannel, byte incomingNumber, byte incomingValue){
  if (isRegisterModule) {
  //Generaal Reset
    if ((incomingValue == 127) && (incomingNumber == controlChangeUit)) {
      for (int i = 1; i < totaalModuleKanalen; i++) {
        setOutput(i,LOW);
      }
    }
  //Alle registers los
    if ((incomingValue == 127) && (incomingNumber == controlChangeAan)) {
      for (int i = 1; i < totaalModuleKanalen; i++) {
        setOutput(i,HIGH);
      }
    }
  //Register inschakelen      
    if (incomingNumber == controlChangeAan) {
      if (!(incomingValue<registerOffSet)){ //checkt of dit register binnen ingestelde bereik valt
        if (registerOffSet){ 
            incomingValue=(incomingValue - registerOffSet);  //converteert control change waarde naar juiste output in geval van offset
           }
        setOutput(incomingValue, HIGH);
      }
  }
    //Register uitschakelen
    if (incomingNumber == controlChangeUit) {
      if (!(incomingValue<registerOffSet)) {
        if (registerOffSet){
          incomingValue=(incomingValue - registerOffSet); 
          }
        setOutput(incomingValue, LOW);
      }
    }
  }   
}

void setup() {
  //display init
  display_I2C.begin(DISPLAY_I2C_SDA, DISPLAY_I2C_SCL, 100000);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("SSD1306 allocation failed");
    for(;;); // Don't proceed, loop forever
  } 
  pinMode(IO_Identity,INPUT);
  isOutputModule = digitalRead(IO_Identity); //read hardware type

  //first little text test
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  if (isOutputModule){
    display.println("Miditronics Output");}
  else{
    display.println("Miditronics Input");}
  display.display(); 

  //USB serial init
#ifdef SERIALDEBUG
  Serial.begin(115200);
  Serial.println("debug mode on");
#endif

  //Non-volatile storage init
  NVS.begin();
  loadNVSSettings();
  display.printf("MIDI CH:%02d|module:%d\noffset:%02d |note: %02d\n",MidiChannel,isRegisterModule,registerOffSet,startNoot);
  display.display(); 

  //extenders init
  extendersI2Cinit();
  totaalModuleKanalen = extendersCount()*16;
  extendersInit(totaalModuleKanalen,isOutputModule);
  display.printf("found %d GPIO\n",totaalModuleKanalen);
  display.display();  

  #ifdef SERIALDEBUG
  display.printf("SERIAL DEBUG ON\n");
  display.display();
  #endif

  //Midi init, listen Omni
  //Serial2.begin(31250, SERIAL_8N1, MIDI_IN_RX_PIN, MIDI_IN_TX_PIN); //volgens mij wordt dit al gedaan in de midi.begin
  pinMode(MIDI_IN_DE_PIN, OUTPUT);
  if (isOutputModule){
    digitalWrite(MIDI_IN_DE_PIN, LOW);} //Receiver enable 
  else{
    digitalWrite(MIDI_IN_DE_PIN, HIGH);} //transmitter enable
  MIDI.begin(MidiChannel); //luister/zend op opgegeven kanaal
  Serial2.begin(31250, SERIAL_8N1, MIDI_IN_RX_PIN, MIDI_IN_TX_PIN); //volgens mij wordt dit al gedaan in de midi.begin
#ifdef SERIALMIDI
  Serial.begin(115200);
#endif
  MIDI.turnThruOff();
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandleControlChange(handleControlChange);
 
  delay(2000);
}

void loop() {
  delay(1);
  //handle incoming midi messages
  if (isOutputModule){
    MIDI.read();} //read incoming messages and let handler do the rest
  
  else{//must be input module
    previousInputs[0] = actualInputs[0];//kan vast met een kortere assignment
    previousInputs[1] = actualInputs[1];
    previousInputs[2] = actualInputs[2];
    previousInputs[3] = actualInputs[3];
    readInputs(totaalModuleKanalen, actualInputs);     // TODO make 32 input compatible
    
    for (int i=0;i<4;i++){ //go through 4 input buffers
      uint16_t bitsOn  = ~previousInputs[i] &  actualInputs[i]; //check for new bits high
      uint16_t bitsOff =  previousInputs[i] & ~actualInputs[i]; //check for new bits low
      if (bitsOn){
        for (int j=0;j<16;j++){ //go though 16 bits in input buffer
          if (bitsOn & (1<<j)){
            uint8_t GPIO = bitToGPIO(j+(16*i));
            MIDI.sendNoteOn((GPIO-1)+startNoot,127,MidiChannel);
            #ifdef SERIALDEBUG
            Serial.print(GPIO);
            Serial.println(" on");
            #endif
          }
        }
      }
      if (bitsOff){
       for (int j=0;j<16;j++){ //go though 16 bits in input buffer
          if (bitsOff & (1<<j)){
            uint8_t GPIO = bitToGPIO(j+(16*i));
            MIDI.sendNoteOff((GPIO-1)+startNoot,127,MidiChannel);
            #ifdef SERIALDEBUG
            Serial.print(GPIO);
            Serial.println(" off");
            #endif
          }
        }
      }
    }
    // TODO: send not changes for the changed inputs
  }
}