/*******************************************************************************
 *
 *   Nunchuck as moving light controller LightHack
 *   
 *   https://github.com/ETCLabs/lighthack
 *
 *   (c) 2018 ETC &James Bithell
 *
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <OSCBoards.h>
#include <OSCBundle.h>
#include <OSCData.h>
#include <OSCMatch.h>
#include <OSCMessage.h>
#include <OSCTiming.h>
#ifdef BOARD_HAS_USB_SERIAL
#include <SLIPEncodedUSBSerial.h>
SLIPEncodedUSBSerial SLIPSerial(thisBoardsSerialUSB);
#else
#include <SLIPEncodedSerial.h>
SLIPEncodedSerial SLIPSerial(Serial);
#endif
#include <string.h>

#include <Wire.h>
#include "WiiLib.h"

WiiChuck chuck = WiiChuck(); //http://playground.arduino.cc/Main/WiiChuckClass
 

 /*******************************************************************************
  * Macros and Constants
  ******************************************************************************/
#define SUBSCRIBE    ((int32_t)1)
#define UNSUBSCRIBE    ((int32_t)0)

#define EDGE_DOWN    ((int32_t)1)
#define EDGE_UP      ((int32_t)0)

#define OSC_BUF_MAX_SIZE  512

const String HANDSHAKE_QUERY = "ETCOSC?";
const String HANDSHAKE_REPLY = "OK";

//See displayScreen() below - limited to 10 chars (after 6 prefix chars)
const String VERSION_STRING = "1.0.0.5";

// Change these values to alter how long we wait before sending an OSC ping
// to see if Eos is still there, and then finally how long before we
// disconnect and show the splash screen
// Values are in milliseconds
#define PING_AFTER_IDLE_INTERVAL  2500
#define TIMEOUT_AFTER_IDLE_INTERVAL 5000

/*******************************************************************************

 * Global Variables
 ******************************************************************************/
unsigned long zLastPressed, cLastPressed;
unsigned long lastMessageRxTime = 0;
bool timeoutPingSent = false;

/*******************************************************************************
 * Given an unknown OSC message we check to see if it's a handshake message.
 * If it's a handshake we issue a subscribe, otherwise we begin route the OSC
 * message to the appropriate function.
 *
 * Parameters:
 *  msg - The OSC message of unknown importance
 *
 * Return Value: void
 *
 ******************************************************************************/
void parseOSCMessage(String& msg)
{
  // check to see if this is the handshake string
  if (msg.indexOf(HANDSHAKE_QUERY) != -1)
  {
    // handshake string found!
    SLIPSerial.beginPacket();
    SLIPSerial.write((const uint8_t*)HANDSHAKE_REPLY.c_str(), (size_t)HANDSHAKE_REPLY.length());
    SLIPSerial.endPacket();
  }
}

/*******************************************************************************
 * Sends a message to Eos informing them of a joystick movement.
 *
 ******************************************************************************/
void sendWheelMove(int x, int y)
{
  String wheelMsgA = "/eos/wheel";
  String wheelMsgB = "/eos/wheel";

  if (chuck.buttonZ && chuck.buttonC) {
    wheelMsgA.concat("/fine");
    wheelMsgB.concat("/fine");
    x = x*3; //Calibration Factor for fine
    y = y*1.5; //Calibration Factor  - Y has smaller range on most moving lights
  } else {
    wheelMsgA.concat("/coarse");
    wheelMsgB.concat("/coarse");
    x = x*0.3; //Calibration Factor for coarse
    y = y*0.15; //Calibration Factor - Y has smaller range on most moving lights
  }
  wheelMsgA.concat("/pan");
  wheelMsgB.concat("/tilt");
  
  OSCMessage wheelUpdateA(wheelMsgA.c_str());
  wheelUpdateA.add(x);
  SLIPSerial.beginPacket();
  wheelUpdateA.send(SLIPSerial);
  SLIPSerial.endPacket();

  OSCMessage wheelUpdateB(wheelMsgB.c_str());
  wheelUpdateB.add(y);
  SLIPSerial.beginPacket();
  wheelUpdateB.send(SLIPSerial);
  SLIPSerial.endPacket();
}

/*******************************************************************************
 * Sends a message to Eos informing them of a key press.
 *
 * Parameters:
 *  down - whether a key has been pushed down (true) or released (false)
 *  key - the key that has moved
 *
 * Return Value: void
 *
 ******************************************************************************/
void sendKeyPress(String key)
{
  key = "/eos/key/" + key;
  OSCMessage keyMsg(key.c_str());
  keyMsg.add(EDGE_DOWN);
  SLIPSerial.beginPacket();
  keyMsg.send(SLIPSerial);
  SLIPSerial.endPacket();
  
  OSCMessage keyMsgUp(key.c_str());
  keyMsgUp.add(EDGE_UP);
  SLIPSerial.beginPacket();
  keyMsgUp.send(SLIPSerial);
  SLIPSerial.endPacket();
  
}

/*******************************************************************************
 * Checks the status of all the buttons relevant to Eos (i.e. Next & Last)
 *
 * Parameters: none
 *
 * Return Value: void
 *
 ******************************************************************************/
void checkButtons()
{
  if (chuck.buttonZ && !chuck.buttonC && zLastPressed < (millis()-500)) {
    delay(1000);
    if (!chuck.buttonC) { //If after 300 seconds the C button isn't depressed it means this isn't a false trigger as they try and push both buttons together - so go ahead and send a keypress
      sendKeyPress("NEXT");
      zLastPressed = millis();
    }
  }
  if (chuck.buttonC && !chuck.buttonZ && cLastPressed < (millis()-500)) {
    delay(1000);
    if (!chuck.buttonZ) { //If after 500 seconds the Z button isn't depressed it means this isn't a false trigger as they try and push both buttons together - so go ahead and send a keypress
      sendKeyPress("LAST");
      cLastPressed = millis();
    }
  }
  if (chuck.buttonC && chuck.buttonZ) {
    zLastPressed = millis()+200;
    cLastPressed = millis()+200; //Make an even bigger delay after you release both keys to stop it jumping straight to a button 
  }
}

/*******************************************************************************
 * Here we setup our various input devices. We also prepare
 * to communicate OSC with Eos by setting up SLIPSerial. Once we are done with
 * setup() we pass control over to loop() and never call setup() again.
 *
 * NOTE: This function is the entry function. This is where control over the
 * Arduino is passed to us (the end user).
 *
 ******************************************************************************/
void setup()
{
  SLIPSerial.begin(115200);
  // This is a hack around an Arduino bug. It was taken from the OSC library
  //examples
#ifdef BOARD_HAS_USB_SERIAL
  while (!SerialUSB);
#else
  while (!Serial);
#endif

  // This is necessary for reconnecting a device because it needs some time
  // for the serial port to open, but meanwhile the handshake message was
  // sent from Eos
  SLIPSerial.beginPacket();
  SLIPSerial.write((const uint8_t*)HANDSHAKE_REPLY.c_str(), (size_t)HANDSHAKE_REPLY.length());
  SLIPSerial.endPacket();

  chuck.begin();
  chuck.update();
  zLastPressed = millis();
  cLastPressed = millis();
  chuck.calibrateJoy(); //Calibrate the 0 of the joystick
}

/*******************************************************************************
 * Here we service, monitor, and otherwise control all our peripheral devices.
 * First, we retrieve the status of our encoders and buttons and update Eos.
 * Next, we check if there are any OSC messages for us.
 * Finally, we update our display (if an update is necessary)
 *
 * NOTE: This function is our main loop and thus this function will be called
 * repeatedly forever
 *
 ******************************************************************************/

void loop()
{
  static String curMsg;
  int size;

  delay(20); //Try not to overcheck on the Chuck
  chuck.update(); //Request position of buttons & joystick
  checkButtons();

  // check satus of chuck joystick
  if (chuck.readJoyX() != 0 || chuck.readJoyY() != 0) {
    sendWheelMove(chuck.readJoyX(), chuck.readJoyY());
  }

  // Then we check to see if any OSC commands have come from Eos
  // and update the display accordingly.
  size = SLIPSerial.available();
  if (size > 0)
  {
    // Fill the msg with all of the available bytes
    while (size--)
      curMsg += (char)(SLIPSerial.read());
  }
  if (SLIPSerial.endofPacket())
  {
    parseOSCMessage(curMsg);
    lastMessageRxTime = millis();
    // We only care about the ping if we haven't heard recently
    // Clear flag when we get any traffic
    timeoutPingSent = false;
    curMsg = String();
  }

  if(lastMessageRxTime > 0) 
  {
    unsigned long diff = millis() - lastMessageRxTime;
    //We first check if it's been too long and we need to time out
    if(diff > TIMEOUT_AFTER_IDLE_INTERVAL) 
    {
      lastMessageRxTime = 0;
      timeoutPingSent = false;
    }

    //It could be the console is sitting idle. Send a ping once to
    // double check that it's still there, but only once after 2.5s have passed
    if(!timeoutPingSent && diff > PING_AFTER_IDLE_INTERVAL) 
    {
        OSCMessage ping("/eos/ping");
        ping.add("arduinoHello"); // This way we know who is sending the ping
        SLIPSerial.beginPacket();
        ping.send(SLIPSerial);
        SLIPSerial.endPacket();
        timeoutPingSent = true;
    }
  }

}
