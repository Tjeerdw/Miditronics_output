#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "config.h"
#include "outputs.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <menu.h>
#include <menuIO/adafruitGfxOut.h>
#include <menuIO/keyIn.h>

using namespace Menu;

TwoWire display_I2C =  TwoWire(0);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &display_I2C, OLED_RESET);

keyMap joystickBtn_map[] = {
  { -BUT_RIGHT, defaultNavCodes[enterCmd].ch} ,
  { -BUT_UP, defaultNavCodes[upCmd].ch} ,
  { -BUT_DOWN, defaultNavCodes[downCmd].ch}  ,
  { -BUT_LEFT, defaultNavCodes[escCmd].ch}  ,
};
keyIn<TOTAL_NAV_BUTTONS> joystickBtns(joystickBtn_map);//the input driver

result showEvent(eventMask e,navNode& nav,prompt& item) {
  Serial.print(F("event:"));
  Serial.print(e);
  return proceed;
}

int ledCtrl=LOW;
result myLedOn() {
  setOutput(1,1);
  return proceed;
}
result myLedOff() {
  setOutput(1,0);
  return proceed;
}

result alert(menuOut& o,idleEvent e);
result doAlert(eventMask e, prompt &item);



TOGGLE(ledCtrl,setLed,"Led: ",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("On",HIGH,doNothing,noEvent)
  ,VALUE("Off",LOW,doNothing,noEvent)
);

const char* constMEM hexDigit MEMMODE="0123456789ABCDEF";
const char* constMEM hexNr[] MEMMODE={"0","x",hexDigit,hexDigit};
char buf1[]="0x11";

int test=55;

MENU(mainMenu,"Main menu",doNothing,noEvent,wrapStyle
  ,FIELD(test,"Test","%",0,100,10,1,doNothing,noEvent,wrapStyle)
  ,SUBMENU(setLed)
  ,OP("LED On",myLedOn,enterEvent)
  ,OP("LED Off",myLedOff,enterEvent)
  ,OP("Alert test",doAlert,enterEvent)
  ,EDIT("Hex",buf1,hexNr,doNothing,noEvent,noStyle)
  ,EXIT("<Back")
);

const colorDef<uint16_t> colors[6] MEMMODE={
  {{WHITE,BLACK},{WHITE,BLACK,BLACK}},//bgColor
  {{BLACK,WHITE},{BLACK,WHITE,WHITE}},//fgColor
  {{BLACK,WHITE},{BLACK,WHITE,WHITE}},//valColor
  {{BLACK,WHITE},{BLACK,WHITE,WHITE}},//unitColor
  {{BLACK,WHITE},{WHITE,WHITE,WHITE}},//cursorColor
  {{BLACK,WHITE},{WHITE,BLACK,BLACK}},//titleColor
};

MENU_OUTPUTS(out,MAX_DEPTH
  ,ADAGFX_OUT(display,colors,fontX,fontY,{0,0,SCREEN_WIDTH/fontX,SCREEN_HEIGHT/fontY})
  ,NONE
);

NAVROOT(nav,mainMenu,MAX_DEPTH,joystickBtns,out);

result alert(menuOut& o,idleEvent e) {
  if (e==idling) {
    o.setCursor(0,0);
    o.print(F("alert test"));
    o.setCursor(0,1);
    o.print(F("press [select]"));
    o.setCursor(0,2);
    o.print(F("to continue..."));
  }
  return proceed;
}

result doAlert(eventMask e, prompt &item) {
  nav.idleOn(alert);
  return proceed;
}

result idle(menuOut& o,idleEvent e) {
  o.setCursor(0,0);
  o.print(F("suspended..."));
  o.setCursor(0,1);
  o.print(F("press [select]"));
  o.setCursor(0,2);
  o.print(F("to continue"));
  return proceed;
}


void setup() {
  //USB serial init
  Serial.begin(115200);
  joystickBtns.begin();
  nav.idleTask=idle;//point a function to be used when menu is suspended


  //display init
  display_I2C.begin(DISPLAY_I2C_SDA, DISPLAY_I2C_SCL, 100000);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("SSD1306 allocation failed");
    for(;;); // Don't proceed, loop forever
  } 

  //extenders init
  extendersInit();
    
  //first little text test
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(30, 10);
  display.println("Hello     Church!");
  display.display(); 
}

void loop() {

  nav.doInput();
  if (nav.changed(0)) {//only draw if changed
    nav.doOutput();
    display.display();
  } 
}