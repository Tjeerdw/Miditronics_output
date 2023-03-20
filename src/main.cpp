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
#include <ezButton.h>
 //test
//config midi instance on serial 2
#ifdef SERIALMIDI
  MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);
#else  
  MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);
#endif

//Persistent variables through EEPROM
uint8_t MidiChannel=1;          

enum moduletypes { Noten, Register };
char moduletypeNames[2][6] = { "Noten", "Regis"};
moduletypes moduletype = Noten;

bool isOutputModule = false;
bool MenuActive = false;
bool MenuEditActive = false;
bool menuButtonState = true;
bool menuButtonlastState = true;
unsigned long lastDebounceTime = 0;
unsigned long lastMenuTime = 0;
unsigned long debounceDelay = 50; 
unsigned long menuTimout = 10000;

uint8_t standaardVelocity = 127; // ter ere van hendrikus
uint8_t registerOffSet = 0;     // registeroffset in geval van extra registermodule
uint8_t startNoot = 23;         // midi-nootnummer waarop deze module moet starten (24 = C1, 36 = C2, 48 = C3) 
char noteNames[128][5] = { "C-1","C#-1","D-1","D#-1","E-1","F-1","F#-1","G-1","G#-1","A-1","A#-1","B-1",
                            "C0","C#0","D0","D#0","E0","F0","F#0","G0","G#0","A0","A#0","B0",
                            "C1","C#1","D1","D#1","E1","F1","F#1","G1","G#1","A1","A#1","B1",
                            "C2","C#2","D2","D#2","E2","F2","F#2","G2","G#2","A2","A#2","B2",
                            "C3","C#3","D3","D#3","E3","F3","F#3","G3","G#3","A3","A#3","B3",
                            "C4","C#4","D4","D#4","E4","F4","F#4","G4","G#4","A4","A#4","B4",
                            "C5","C#5","D5","D#5","E5","F5","F#5","G5","G#5","A5","A#5","B5",
                            "C6","C#6","D6","D#6","E6","F6","F#6","G6","G#6","A6","A#6","B6",
                            "C7","C#7","D7","D#7","E7","F7","F#7","G7","G#7","A7","A#7","B7",
                            "C8","C#8","D8","D#8","E8","F8","F#8","G8","G#8","A8","A#8","B8",
                            "C9","C#9","D9","D#9","E9","F9","F#9","G9",};


//lijst met registernamen. Deze bevat niet toegewezen registernummers, vervangen door 'DMMY' t.b.v kloppende indexering
char registerNames[87][20] = {"Prestant 16'","Bourdon 16'","Principaal 8'","Bourdon 8'","Salicionaal 8'",
                            "Quintadeen 8'","Flute Harmonique 8'","Octaaf 4'","Fluit 4'","Salicet 4'",
                            "Quint 2 2/3'","Octaaf 2'","Nachthoorn 2'","Tertiaan II","Cymbel III",
                            "Mixtuur III-V","Cornet V ","Fagot 16'","Trompet 8'","Dulciaan 8'",
                            "Klaroen 4'","DMMY","DMMY","DMMY","DMMY","DMMY","DMMY","DMMY","DMMY",
                            "Quintadeen 16'","Prestant 8'","Holpijp 8'","Violon 8'","Unda Maris 8'",
                            "Prestant 4'","Roerfluit 4'","Salicionaal 4'","Nasard 2 2/3'","Octaaf 2'",
                            "Woudfluit 2'","Quint 1 1/3'","Sesquialter II","Scherp III","Trompet Harm. 8'",
                            "Schalmeij 8'","DMMY","DMMY","DMMY","DMMY","Viola di Gamba 8'","Voix Celeste 8'",
                            "Bourdon 8'","Prestantfluit 4'","Aeoline 4'","Roerquint 2 2/3'","Piccolo 2'",
                            "Terts 1 3/5'","Larigot 1 1/3'","Septime 1 1/7'","Flageolet 1'","Carillon",
                            "Basson Hobo 8'","Clarinet 8'","Vox Humana 8'","DMMY","DMMY","DMMY","DMMY","DMMY",
                            "Prestant 8'","Roerfluit 8'","Quintadeen  8'","Viola 8'","Octaaf  4'","Gemshoorn 4'",
                            "Fluit douce 4'","Nasard 2 2/3'","Septime 2 2/7'","Octaaf 2'","Flageolet 2'",
                            "Terts 1 3/5'","Sifflet 1'","None 8/9'","Ruispijp II","Mixtuur III-V","Regaal 16'","Kromhoorn 8'"};


const int controlChangeAan = 80;//control change waarde aan
const int controlChangeUit = 81;//control change waarde uit
uint8_t totaalModuleKanalen = 0;   //definieert het aantal kanalen dat deze module kan aansturen (32/64)
uint16_t actualInputs[4] = {0,0,0,0};
uint16_t previousInputs[4] = {0,0,0,0};
#define eindNoot  startNoot+totaalModuleKanalen //midi-nootnummer waar deze module stopt met reageren
uint8_t menuCounter = 1;
uint8_t numberOfMenuItems = 4;

ezButton buttonLeft(BUT_LEFT_PIN);
ezButton buttonRight(BUT_RIGHT_PIN);
ezButton buttonUp(BUT_UP_PIN);
ezButton buttonDown(BUT_DOWN_PIN);

TwoWire display_I2C =  TwoWire(0);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &display_I2C, OLED_RESET);

void loadNVSSettings(){
  MidiChannel = NVS.getInt("channel");
  moduletype = (moduletypes)NVS.getInt("regmodule"); //TODO: test this
  registerOffSet = NVS.getInt("regoffset");
  startNoot = NVS.getInt("startnoot");
  
}

void saveNVSSettingsReset(){
  NVS.setInt("channel",MidiChannel);
  NVS.setInt("regmodule",moduletype);
  NVS.setInt("regoffset",registerOffSet);
  NVS.setInt("startnoot", startNoot);
  ESP.restart();
}

void writeIdleScreen(){
  display.clearDisplay();
  display.setTextSize(4);
  display.setCursor(0,0);
  display.printf("CH:%02d\n",MidiChannel);
  display.display();
}

void writeNoteOnScreen(uint8_t lastnote){
  display.setTextSize(4);
  display.setCursor(0,32);
  display.print("    ");
  display.setCursor(0,32);
  display.print(noteNames[lastnote]);
  display.display();
}


void writeRegisterOnScreen (uint8_t lastRegister){
  display.setTextSize(2);
  display.setCursor(0,32);
  display.print("    ");
  display.setCursor(0,32);
  display.print(registerNames[lastRegister]);
  display.display();
}


//note-On message afhandelen
void handleNoteOn(byte incomingChannel, byte pitch, byte velocity){
  if (moduletype==Noten) {
    if ((pitch>=startNoot) && (pitch<=eindNoot)) {
      int outputpitch = (pitch-(startNoot-1)); //converteert noot naar het juiste outputnummer        
      setOutput(outputpitch,HIGH); //schakel noot in
      if (!MenuActive){
        writeNoteOnScreen(pitch);
      }
    }
  }
}

//note-Off message afhandelen
void handleNoteOff(byte incomingChannel, byte pitch, byte velocity){
  if (moduletype==Noten) {
    if ((pitch>=startNoot) && (pitch<=eindNoot)) {
      pitch = (pitch-(startNoot-1)); //converteert noot naar het juiste outputnummer
      setOutput(pitch,LOW); //schakel noot uit
    } 
  }
}

//Control-Change message afhandelen
void handleControlChange(byte incomingChannel, byte incomingNumber, byte incomingValue){
  if (moduletype==Register) {
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
          setOutput(incomingValue, HIGH);
          if (!MenuActive){
            writeRegisterOnScreen(incomingValue);
          }
        }
        else {
          setOutput(incomingValue, HIGH);
          if (!MenuActive){
            writeRegisterOnScreen(incomingValue);
          }
        }
      }
    }
    //Register uitschakelen
    if (incomingNumber == controlChangeUit) {
      if (!(incomingValue<registerOffSet)) {
        if (registerOffSet){
          incomingValue=(incomingValue - registerOffSet); 
          setOutput(incomingValue, LOW);
          if (!MenuActive){
          writeRegisterOnScreen(incomingValue);
          }
        }
        else {
          setOutput(incomingValue, LOW);
          if (!MenuActive){
          writeRegisterOnScreen(incomingValue);
          }
        }
      }
    }
  }   
}

void drawMenu(){
  lastMenuTime = millis();

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(7,0);
  display.print("Midi kanaal");
  display.setCursor(95,0);
  display.print(MidiChannel);

  display.setCursor(7,8);
  display.print("Startnoot"); //todo alternate register setting
  display.setCursor(95,8);
  display.print(noteNames[startNoot]);

  display.setCursor(7,16);
  display.print("Module type");
  display.setCursor(95,16);
  display.print(moduletypeNames[moduletype]);

  display.setCursor(7,24);
  display.print("Save & reboot");

  display.setCursor(((int)MenuEditActive*88),(menuCounter-1)*8);
  display.print(">");
  display.display();  
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
  display.setTextColor(WHITE,BLACK);
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
  display.printf("MIDI kanaal: %02d\nModuletype: %s\nRegister offset: %02d\nStartnoot: %s\n",MidiChannel,moduletypeNames[moduletype],registerOffSet,noteNames[startNoot]);
  display.display(); 

  //extenders init
  extendersI2Cinit();
  totaalModuleKanalen = extendersCount()*16;
  extendersInit(totaalModuleKanalen,isOutputModule);
  display.printf("found %d GPIO\n",totaalModuleKanalen);
  display.display();  

  #ifdef SERIALDEBUG
  display.printf("!!!SERIAL DEBUG ON!!!\n");
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
 
  delay(3000);
  writeIdleScreen();
}

void loop() {
 buttonRight.loop();

  if (buttonRight.isPressed()){
      MenuActive = true;
      lastMenuTime = millis();
      drawMenu();
    }

  while(MenuActive){ //just stay in the menu, nothing else to do
    buttonDown.loop();
    buttonUp.loop();
    buttonLeft.loop();
    buttonRight.loop();

    if (millis() - lastMenuTime > menuTimout){
      MenuActive = false; //exit menu
      menuCounter = 1;
      MenuEditActive = false;
      writeIdleScreen(); 
    }
    
    if (MenuEditActive){
      if (buttonLeft.isPressed()){
      MenuEditActive = false;
      drawMenu();
      }
      else if (buttonUp.isPressed()){
        switch (menuCounter)
        {
        case 1:
          if (MidiChannel<16){
            MidiChannel++;
            drawMenu();
          }          
          break;
        case 2:
          if (startNoot<(127-totaalModuleKanalen)){
            startNoot++;
            drawMenu();
          }
          break;
        case 3:
          moduletype = Noten;
          drawMenu();
          break;            
        default:
          break;
        }
      }
      else if (buttonDown.isPressed()){
        switch (menuCounter)
        {
        case 1:
          if (MidiChannel>0){
            MidiChannel--;
            drawMenu();
          }          
          break;
        case 2:
          if (startNoot>0){
            startNoot--;
            drawMenu();
          }
          break;
        case 3:
          moduletype = Register;  
          drawMenu();
          break;          
        default:
          break;
        }
      }
    }

    else{ //must be main menu
      if (buttonDown.isPressed()){
        menuCounter++;
        lastMenuTime = millis();
        if(menuCounter > numberOfMenuItems){
          menuCounter = 1;
        }
        drawMenu();
      }
      else if (buttonUp.isPressed()){
        menuCounter--;
        lastMenuTime = millis();
        if(menuCounter < 1){
          menuCounter = numberOfMenuItems;
        }
        drawMenu();
      }
      else if (buttonRight.isPressed()){
        if (menuCounter == 4){
          saveNVSSettingsReset();
        }
        MenuEditActive = true;
        drawMenu();
      }
      else if (buttonLeft.isPressed()){
        MenuActive = false; //exit menu
        menuCounter = 1;
        writeIdleScreen(); 
      }
    }
  }

  if (isOutputModule){ //handle incoming midi messages
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
            if (moduletype == Register){
              MIDI.sendControlChange((GPIO-1)+startNoot,controlChangeAan,MidiChannel);
              writeRegisterOnScreen((GPIO-1)+startNoot);
            }
            else{
              MIDI.sendNoteOn((GPIO-1)+startNoot,standaardVelocity,MidiChannel);
              writeNoteOnScreen((GPIO-1)+startNoot);
            }
            
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
            if (moduletype == Register){
              MIDI.sendControlChange((GPIO-1)+startNoot,controlChangeUit,MidiChannel);
            }
            else{
              MIDI.sendNoteOff((GPIO-1)+startNoot,standaardVelocity,MidiChannel);
            }
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