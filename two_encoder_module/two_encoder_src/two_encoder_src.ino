// Copyright (c) 2017 Electronic Theatre Controls, Inc., http://www.etcconnect.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE. 


/*******************************************************************************
 *
 *   Electronic Theatre Controls
 *
 *   LightHack_ - Two Encoder Three Button Module
 *
 *   (c) 2017 by ETC
 *
 *
 *   This code implements a Pan/Tilt module using two encoders and three
 *   buttons. The two encoders function as pan and tilt controllers with one
 *   button being reserved for controlling fine/coarse mode. The other two
 *   buttons are assigned to Next and Last which make it easy to switch between
 *   channels.
 *
 *******************************************************************************
 *
 *  Revision History
 *
 *  yyyy-mm-dd   Vxx     By_Who                 Comment
 *
 *  2017-07-21   V1.000  Ethan Oswald Massey    Original creation
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
#include <SLIPEncodedSerial.h>
#include <LiquidCrystal.h>
#include <string.h>

 /*******************************************************************************
  * Macros and Constants
  ******************************************************************************/
#define LCD_CHARS   16
#define LCD_LINES   2   // Currently assume at least 2 lines

#define PAN_POS     4
#define TILT_POS    5
#define NEXT_BTN    6
#define LAST_BTN    7
#define SHIFT_BTN   8

#define SUBSCRIBE   1
#define UNSUBSCRIBE 0

#define EDGE_DOWN   1
#define EDGE_UP     0

// These define which direction is "forward" for an encoder
#define FORWARD     0
#define REVERSE     1

// Change these values to switch which direction increase/decrease pan/tilt
#define PAN_DIR     FORWARD
#define TILT_DIR    FORWARD

#define SIG_DIGITS  3   // Number of significant digits displayed

#define OSC_BUF_MAX_SIZE 512

const String HANDSHAKE_QUERY = "ETCOSC?";
const String HANDSHAKE_REPLY = "OK";

const String EOS_KEY = "/eos/key/";
const String EOS_NEXT_KEY = "NEXT";
const String EOS_LAST_KEY = "LAST";

const String EOS_WHEEL = "/eos/wheel";
const String EOS_PT_ADDRESS = "/eos/out/pantilt";

/*******************************************************************************
 * Local Types
 ******************************************************************************/
enum WHEEL_TYPE { TILT, PAN };
enum WHEEL_MODE { COARSE, FINE };

struct Encoder
{
    uint8_t pinA;
    uint8_t pinB;
    uint8_t buttonPin;
    uint8_t buttonState;
    int pinAPrevious;
    int pinBPrevious;
    float pos;
    int direction;
};
struct Encoder panWheel;
struct Encoder tiltWheel;

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
SLIPEncodedSerial SLIPSerial(Serial);

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

bool updateDisplay = false;


/*******************************************************************************
 * Local Functions
 ******************************************************************************/

/*******************************************************************************
 * Issues all our subscribes to Eos. When subscribed, Eos will keep us updated
 * with the latest values for a given parameter.
 *
 * Parameters:  none
 *
 * Return Value: void
 *
 ******************************************************************************/
void issueSubscribes()
{
    // subscribe to Eos pan & tilt updates
    OSCMessage subPan("/eos/subscribe/param/pan");
    subPan.add(SUBSCRIBE);
    SLIPSerial.beginPacket();
    subPan.send(SLIPSerial);
    SLIPSerial.endPacket();

    OSCMessage subTilt("/eos/subscribe/param/tilt");
    subTilt.add(SUBSCRIBE);
    SLIPSerial.beginPacket();
    subTilt.send(SLIPSerial);
    SLIPSerial.endPacket();
}

/*******************************************************************************
 * Given a valid OSCMessage (relavent to Pan/Tilt), we update our Encoder struct
 * with the new position information.
 *
 * Parameters:
 *  msg - The OSC message we will use to update our internal data
 *  addressOffset - Unused (allows for mulitple nested roots)
 *
 * Return Value: void
 *
 ******************************************************************************/
void parseWheelUpdate(OSCMessage& msg, int addressOffset)
{
    panWheel.pos  = msg.getOSCData(PAN_POS)->getFloat();
    tiltWheel.pos = msg.getOSCData(TILT_POS)->getFloat();

    updateDisplay = true;
}

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

        // Let Eos know we want updates on some things
        issueSubscribes();
    }
    else
    {
        // prepare the message for routing by filling an OSCMessage object with our message string
        OSCMessage oscmsg;
        oscmsg.fill((uint8_t*)msg.c_str(), (int)msg.length());
        // route pan/tilt messages to the parseWheelUpdate function
        oscmsg.route(EOS_PT_ADDRESS.c_str(), parseWheelUpdate);
    }
}

/*******************************************************************************
 * Updates the display with the latest pan and tilt positions.
 *
 * Parameters:  none
 *
 * Return Value: void
 *
 ******************************************************************************/
void displayStatus()
{
    lcd.clear();
    // put the cursor at the begining of the first line
    lcd.setCursor(0, 0);
    lcd.print("Pan:  ");
    lcd.print(panWheel.pos, SIG_DIGITS);

    // put the cursor at the begining of the second line
    lcd.setCursor(0, 1);
    lcd.print("Tilt: ");
    lcd.print(tiltWheel.pos, SIG_DIGITS);

    updateDisplay = false;
}

/*******************************************************************************
 * Initalizes a given encoder struct to the requested parameters.
 *
 * Parameters:
 *  encoder - Pointer to the encoder we will be inializing
 *  pinA - Where the A pin is connected to the arduino
 *  pinB - Where the B pin is connected to the arduino
 *  buttonPin - Where the button pin is connected to the arduino
 *  direction - Determines if clockwise or counterclockwise is "forward"
 *
 * Return Value: void
 *
 ******************************************************************************/
void initEncoder(struct Encoder* encoder, int32_t pinA, int32_t pinB, int32_t buttonPin, int direction)
{
    encoder->pinA = pinA;
    encoder->pinB = pinB;
    encoder->buttonPin = buttonPin;
    encoder->pos = 0;
    encoder->direction = direction;

    pinMode(pinA, INPUT_PULLUP);
    pinMode(pinB, INPUT_PULLUP);
    pinMode(buttonPin, INPUT_PULLUP);

    encoder->pinAPrevious = digitalRead(pinA);
    encoder->pinBPrevious = digitalRead(pinB);
    encoder->buttonState = digitalRead(buttonPin);
}

/*******************************************************************************
 * Checks if the encoder has moved by comparing the previous state of the pins
 * with the current state. If they are different, we know there is movement.
 * In the event of movement we update the current state of our pins.
 *
 * Parameters:
 *  encoder - Pointer to the encoder we will be checking for motion
 *
 * Return Value:
 *  encoderMotion - Returns the 0 if the encoder has not moved
 *                              1 for forward motion
 *                             -1 for reverse motion
 *
 ******************************************************************************/
int8_t updateEncoder(struct Encoder* encoder)
{
    int8_t encoderMotion = 0;
    int pinACurrent = digitalRead(encoder->pinA);
    int pinBCurrent = digitalRead(encoder->pinB);

    // has the encoder moved at all?
    if (encoder->pinAPrevious != pinACurrent)
    {
        // Since it has moved, we must determine if the encoder has moved forwards or backwards
        encoderMotion = (encoder->pinAPrevious == encoder->pinBPrevious) ? -1 : 1;

        // If we are in reverse mode, flip the direction of the encoder motion
        if (encoder->direction == REVERSE)
            encoderMotion = -encoderMotion;
    }
    encoder->pinAPrevious = pinACurrent;
    encoder->pinBPrevious = pinBCurrent;

    return encoderMotion;
}

/*******************************************************************************
 * Sends a message to Eos informing them of a wheels movement.
 *
 * Parameters:
 *  type - the type of wheel that's moving (i.e. pan or tilt)
 *  ticks - the direction and intensity of the movement
 *
 * Return Value: void
 *
 ******************************************************************************/
void sendWheelMove(WHEEL_TYPE type, float ticks)
{
    String wheelMsg(EOS_WHEEL);

    if (digitalRead(SHIFT_BTN) == LOW)
      wheelMsg.concat("/fine");
    else
      wheelMsg.concat("/coarse");

    if (type == PAN)
        wheelMsg.concat("/pan");
    else if (type == TILT)
        wheelMsg.concat("/tilt");
    else
        // something has gone very wrong
        return;

    OSCMessage wheelUpdate(wheelMsg.c_str());
    wheelUpdate.add(ticks);
    SLIPSerial.beginPacket();
    wheelUpdate.send(SLIPSerial);
    SLIPSerial.endPacket();
}

/*******************************************************************************
 * Sends a message to Eos informing them of a keys press.
 *
 * Parameters:
 *  down - whether a key has been pushed down (true) or released (false)
 *  key - the key that has moved
 *
 * Return Value: void
 *
 ******************************************************************************/
void sendKeyPress(bool down, String key)
{
    key = EOS_KEY + key;
    OSCMessage keyMsg(key.c_str());

    if (down)
        keyMsg.add(EDGE_DOWN);
    else
        keyMsg.add(EDGE_UP);

    SLIPSerial.beginPacket();
    keyMsg.send(SLIPSerial);
    SLIPSerial.endPacket();
}

/*******************************************************************************
 * Checks the status of all the buttons relavent to Eos (i.e. Next & Last)
 *
 * NOTE: This does not check the shift key. The shift key is used in tandom with
 * the encoder to determine coarse/fine mode and thus does not report to Eos
 * directly.
 *
 * Parameters: none
 *
 * Return Value: void
 *
 ******************************************************************************/
void checkButtons()
{
    static int nextKeyState = HIGH;
    static int lastKeyState = HIGH;

    // Has the button state changed
    if (digitalRead(NEXT_BTN) != nextKeyState)
    {
        // Notify Eos of this key press
        if (nextKeyState == LOW)
        {
            sendKeyPress(false, EOS_NEXT_KEY);
            nextKeyState = HIGH;
        }
        else
        {
            sendKeyPress(true, EOS_NEXT_KEY);
            nextKeyState = LOW;
        }
    }

    if (digitalRead(LAST_BTN) != lastKeyState)
    {
        if (lastKeyState == LOW)
        {
            sendKeyPress(false, EOS_LAST_KEY);
            lastKeyState = HIGH;
        }
        else
        {
            sendKeyPress(true, EOS_LAST_KEY);
            lastKeyState = LOW;
        }
    }
}

/*******************************************************************************
 * Here we setup our encoder, lcd, and various input devices. We also prepare
 * to comunicate OSC with Eos by setting up SLIPSerial. Once we are done with
 * setup() we pass control over to loop() and never call setup() again.
 *
 * NOTE: This function is the entry function. This is where control over the
 * arduino is passed to us (the end user).
 *
 * Parameters: none
 *
 * Return Value: void
 *
 ******************************************************************************/
void setup()
{
    SLIPSerial.begin(115200);
// This is a hack around an arduino bug. It was taken from the OSC library examples
#if ARDUINO >= 100
    while (!Serial)
    ;
#endif

    initEncoder(&panWheel, A0, A1, A2, PAN_DIR);
    initEncoder(&tiltWheel, A3, A4, A5, TILT_DIR);

    lcd.begin(LCD_CHARS, LCD_LINES);
    lcd.clear();

    pinMode(NEXT_BTN, INPUT_PULLUP);
    pinMode(LAST_BTN, INPUT_PULLUP);
    pinMode(SHIFT_BTN, INPUT_PULLUP);

    displayStatus();
}

/*******************************************************************************
 * Here we service, monitor, and otherwise control all our peripheral devices.
 * First, we retrieve the status of our encoders and buttons and update Eos.
 * Next, we check if there are any OSC message for us.
 * Finally, we update our display (if an update is necessary)
 *
 * NOTE: This function is our main loop and thus we will loop here forever.
 *
 * Parameters: none
 *
 * Return Value: void
 *
 ******************************************************************************/
void loop()
{
    static String curMsg;
    int size;
    // get the updated state of each encoder
    int32_t panMotion = updateEncoder(&panWheel);
    int32_t tiltMotion = updateEncoder(&tiltWheel);

    // check for next/last updates
    checkButtons();

    // now update our wheels
    if (tiltMotion != 0)
        sendWheelMove(TILT, tiltMotion);

    if (panMotion != 0)
        sendWheelMove(PAN, panMotion);

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
        curMsg = String();
    }

    if (updateDisplay)
      displayStatus();
}

