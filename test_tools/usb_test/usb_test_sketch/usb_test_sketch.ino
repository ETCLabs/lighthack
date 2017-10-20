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
 *   lighthack - USB Test Sketch
 *
 *   (c) 2017 by ETC
 *
 *
 *   This code provides a minimal OSC/USB implementation designed for
 *   troubleshooting issues with the other DIY modules. It does not interface
 *   with any hardware; it simply attempts to connect with Eos. Upon doing so,
 *   it begins periodically sending /eos/ping commands which can then be viewed
 *   in the Eos application to verify that the OSC pipe is working.
 *
 *******************************************************************************
 *
 *  Revision History
 *
 *  yyyy-mm-dd   Vxx        By_Who                 Comment
 *
 *  2017-10-20   V1.0.0.1   Sam Kearney            Original creation
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

 /*******************************************************************************
  * Macros and Constants
  ******************************************************************************/
const String HANDSHAKE_QUERY = "ETCOSC?";
const String HANDSHAKE_REPLY = "OK";

/*******************************************************************************
 * Local Types
 ******************************************************************************/

/*******************************************************************************
 * Global Variables
 ******************************************************************************/

/*******************************************************************************
 * Local Functions
 ******************************************************************************/

/*******************************************************************************
 * Given an unknown OSC message we check to see if it's a handshake message.
 * The handshake message must be replied to for Eos to recognize this device
 * as an OSC device.
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
 * Here we prepare to communicate OSC with Eos by setting up SLIPSerial. Once
 * we are done with setup() we pass control over to loop() and never call
 * setup() again.
 *
 * Parameters: none
 *
 * Return Value: void
 *
 ******************************************************************************/
void setup()
{
    SLIPSerial.begin(115200);
    // This is a hack around an Arduino bug. It was taken from the OSC library
    // examples
#ifdef BOARD_HAS_USB_SERIAL
    while (!SerialUSB);
#else
    while (!Serial);
#endif

    // this is necessary for reconnecting a device because it needs some time
    // for the serial port to open, but meanwhile the handshake message was
    // sent from Eos
    SLIPSerial.beginPacket();
    SLIPSerial.write((const uint8_t*)HANDSHAKE_REPLY.c_str(), (size_t)HANDSHAKE_REPLY.length());
    SLIPSerial.endPacket();
}

/*******************************************************************************
 * Main loop: manage OSC I/O. Send a ping command to Eos every second.
 *
 * Parameters: none
 *
 * Return Value: void
 *
 ******************************************************************************/
void loop()
{
    static String curMsg;
    static unsigned long lastTimeSent;
    static int32_t pingNum;
    unsigned long curTime;
    int size;

    // Check to see if any OSC commands have come from Eos that we need to respond to.
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

    // Send a ping every second.
    curTime = millis();
    if (curTime - lastTimeSent > 1000)
    {
        OSCMessage ping("/eos/ping");
        ping.add(pingNum++);
        SLIPSerial.beginPacket();
        ping.send(SLIPSerial);
        SLIPSerial.endPacket();
        lastTimeSent = curTime;
    }
}

