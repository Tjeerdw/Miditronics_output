#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "config.h"
#include "outputs.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMono9pt7b.h>

#include <menu.h>
#include <menuIO/adafruitGfxOut.h>
#include <menuIO/keyIn.h>

#include <MIDI.h>
//config midi instance on serial 2
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);
//config (temp) variables for midi implementation, some are waiting on menu implementation and eeprom storage
int listeningMidiChannel=1;
boolean registerModule = false;
int controlChangeAan = 80; //control change waarde aan
int controlChangeUit = 81; //control change waarde uit
int totaalModuleKanalen = 64; //definieert het aantal kanalen dat deze module kan aansturen (32/64)
int registerOffSet = 0; //registeroffset in geval van extra registermodule
int startNoot = 36; //midi-nootnummer waarop deze module moet starten (23 = C1, 36 = C2, 49 = C3)
int eindNoot = (startNoot + totaalModuleKanalen); //midi-nootnummer waar deze module stopt met reageren


using namespace Menu;

TwoWire display_I2C =  TwoWire(0);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &display_I2C, OLED_RESET);

keyMap joystickBtn_map[] = {
  { -BUT_RIGHT, defaultNavCodes[enterCmd].ch} ,
  { -BUT_DOWN, defaultNavCodes[upCmd].ch} ,
  { -BUT_UP, defaultNavCodes[downCmd].ch}  ,
  { -BUT_LEFT, defaultNavCodes[escCmd].ch}  ,
};
keyIn<TOTAL_NAV_BUTTONS> joystickBtns(joystickBtn_map);//the input driver

int ledCtrl=LOW;
result myLedOn() {
  setOutput(1,1);
  return proceed;
}
result myLedOff() {
  setOutput(1,0);
  return proceed;
}

TOGGLE(ledCtrl,setOutputType,"type: ",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("Noten",HIGH,doNothing,noEvent)
  ,VALUE("Registers",LOW,doNothing,noEvent)
);


MENU(mainMenu,"Menu",doNothing,noEvent,wrapStyle
  ,FIELD(listeningMidiChannel,"Channel","",1,16,1,0,doNothing,noEvent,wrapStyle)
  ,SUBMENU(setOutputType)
  ,OP("LED On",myLedOn,enterEvent)
  ,OP("LED Off",myLedOff,enterEvent)
  ,EXIT("<Back")
);

const colorDef<uint16_t> colors[6] MEMMODE={
  {{BLACK,WHITE},{BLACK,WHITE,WHITE}},//bgColor
  {{WHITE,BLACK},{WHITE,BLACK,BLACK}},//fgColor
  {{WHITE,BLACK},{WHITE,BLACK,BLACK}},//valColor
  {{WHITE,BLACK},{WHITE,BLACK,BLACK}},//unitColor
  {{WHITE,BLACK},{BLACK,BLACK,BLACK}},//cursorColor
  {{WHITE,BLACK},{BLACK,WHITE,WHITE}},//titleColor
};

MENU_OUTPUTS(out,MAX_DEPTH
  ,ADAGFX_OUT(display,colors,fontX,fontY,{0,0,SCREEN_WIDTH/fontX,SCREEN_HEIGHT/fontY})
  ,NONE
);

NAVROOT(nav,mainMenu,MAX_DEPTH,joystickBtns,out);

result idle(menuOut& o,idleEvent e) {
  o.setCursor(0,0);
  o.print(F("Miditronics Output"));
  o.setCursor(0,1);
  o.print(F("Press button for menu"));
  return proceed;
}

//note-On message afhandelen
void handleNoteOn(byte incomingChannel, byte pitch, byte velocity)
{
  if (!registerModule) {  
    velocity = 127; //ter ere van Hendrikus
    pitch = (pitch-(startNoot-1)); //converteert noot naar het juiste outputnummer        
    setOutput(pitch,HIGH); //schakel noot in
  }
}

//note-Off message afhandelen
void handleNoteOff(byte incomingChannel, byte pitch, byte velocity)
{
  if (!registerModule) {
    velocity = 127; //ter ere van Hendrikus
    if ((pitch>=startNoot) && (pitch<=eindNoot)) {
      pitch = (pitch-(startNoot-1)); //converteert noot naar het juiste outputnummer
      setOutput(pitch,LOW); //schakel noot uit
    } 
  }
}

//Control-Change message afhandelen
void handleControlChange(byte incomingChannel, byte incomingNumber, byte incomingValue)
{
  if (registerModule) {
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
        if (registerOffSet >0){ 
            incomingValue=(incomingValue - registerOffSet);  //converteert control change waarde naar juiste output in geval van offset
           }
        setOutput(incomingValue, HIGH);
      }
  }
    //Register uitschakelen
    if (incomingNumber == controlChangeUit) {
      if (!(incomingValue<registerOffSet)) {
        if (registerOffSet >0){
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

  //first little text test
  display.clearDisplay();
  //display.setFont(&FreeMono9pt7b);
  //display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("Miditronics Output");
  display.println("Booting...");
  display.display(); 

  //USB serial init
  Serial.begin(115200);
  display.println("Serial init");
  display.display(); 

  //Navigation
  joystickBtns.begin();
  nav.timeOut=5;
  nav.idleTask=idle;//point a function to be used when menu is suspended
  display.println("Navigation init");
  display.display();  

  //extenders init
  extendersInit();
  display.println("Extenders init");
  display.display();  

  display.println("Starting...");
  display.display();  
  delay(2000);

  //Midi init, listen Omni
  Serial2.begin(31250, SERIAL_8N1, MIDI_RX_PIN, MIDI_TX_PIN);
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandleControlChange(handleControlChange);
  MIDI.begin(listeningMidiChannel); //luister op opgegeven kanaal



}

void loop() {

  nav.doInput();
  if (nav.changed(0)) {//only draw if changed
    nav.doOutput();
    display.display();
  } 
  //handle incoming midi messages
  MIDI.read();
}