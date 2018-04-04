#  Lighthack Nunchuk Assembly Instructions

<a href="http://www.youtube.com/watch?feature=player_embedded&v=b4YxDby04tY
" target="_blank"><img src="http://img.youtube.com/vi/b4YxDby04tY/0.jpg" 
alt="YouTube Video" width="240" height="180" border="10" /></a>

## Parts List

| **ETC PN** | **Vendor PN** | **Qty** | **Description** |
| --- | --- | --- | --- |
| 4201B9001 | Mouser 782-A000066 | 1 | Arduino Uno R3 |
| W6378 |   | 1 | USB Cable, A to B |
|   | Adafruit 342 | 1 | Nunchuk Remote Controller Attachment For Nintendo Wii |
|   | Adafruit 345 | 1 | Nunchucky (Wii Nunchuck breakout adapter) |
| 4201B7001 |   | 1 | Wire Solid Core 22AWG/0.65mm2 2-3&#39;/60-90cm |
| 4201A4001 | Hammond 1591U | 1 | Enclosure with lid and screws |

## Tools

- Wire strippers and cutters
- Soldering iron and solder

## Software

- Arduino Integrated Development Environment (IDE). Download from [https://www.arduino.cc/en/Main/Software](https://www.arduino.cc/en/Main/Software)
- Arduino sketch for Nunchuk. Download from [https://github.com/ETCLabs/lighthack](https://github.com/ETCLabs/lighthack)
- Arduino OSC library. Download from [https://github.com/CNMAT/OSC](https://github.com/CNMAT/OSC)
- Arduino to Eos Test Application. Download from [https://github.com/ETCLabs/lighthack](https://github.com/ETCLabs/lighthack)

## Test the Software

It is recommended that you test the Arduino before starting the assembly of the enclosure.

1. Download the source code from https://github.com/ETCLabs/lighthack, and extract the .zip file (or clone the repository if you're git-savvy!).

1. Open the Arduino USB Test sketch  (lighthackmaster\test\_tools\usb\_test\usb\_test\_sketch\usb\_test\_sketch.ino) in the Arduino IDE.
1. Before the sketch will compile, we need to add additional code (called a &quot;library&quot;) so that the Arduino knows how to speak OSC. Download the library as a .zip from [https://github.com/CNMAT/OSC](https://github.com/CNMAT/OSC).
1. In the Arduino IDE, select **Sketch &gt; Include Library &gt;Add .ZIP Library...** and select the OSC zip file you downloaded
1. Press the check mark to verify your sketch.
![](https://i.imgur.com/uMSuBz5.png)
1. Connect the Arduino to your computer using the USB cable.
1. The Arduino IDE will automatically detect your Arduino Uno and select it. You can verify this in the Tools menu:
![](https://i.imgur.com/vm8c9vk.png)
1. Press the arrow to upload your sketch to the Arduino.
![](https://i.imgur.com/RToec6g.png)
1. After the sketch has been loaded, the Arduino&#39;s lights should have the L and ON lights solid and the TX light blinking once per second:
![](https://i.imgur.com/4AtZWOc.png)
1. Launch Eos software, open Tab 99 Diagnostics, and look for messages that read:
```
        OSC USB Device Handshake Complete [OK]
        OSC USB Device Handshake Initiated [ETCOSC?]
```


The TX and RX lights should blink in sequence once per second.
Press `Incoming OSC` and you should see repeating messages of `[OSC Packet] /eos/ping` and a number counting up

These messages indicate that your Arduino and Eos are talking to each other.

The RX LED on your Arduino should also start blinking.

11. Exit the Eos software.
1. Now, load the main nunchuk.ino code. If you&#39;ve added the OSC library properly, the verification will be successful.
1. Press the arrow to upload your sketch to the Arduino.
1. After the sketch has been loaded, start Eos again and look for the same Handshake messages. Note that in this mode, the Arduino does not send the repeating Ping messages, and the Arduino&#39;s TX/RX LEDs will only blink once.

If text does not appear, follow the troubleshooting steps at [https://github.com/ETCLabs/lighthack/blob/master/box\_1/box\_1\_assembly\_instructions.pdf](https://github.com/ETCLabs/lighthack/blob/master/box_1/box_1_assembly_instructions.pdf)

## Adding the Nunchuk 

Connect the `3.3v` and `Gnd` pins to the respective Arduino Pins using the jumper wire and soldering iron. Connect Nunchuk `Data` to `SDA` (Analogue 4 on Arduino Uno) and `Clk` to `SCL` (Analogue 5 on Arduino Uno)
![Breakout Adaptor](https://i.imgur.com/DKtQkd5.jpg)
![Wiring Diagram](https://i.imgur.com/ZD6JGy8.png)
Plug in the Nunchuk, and then powerup the Arduino

## Usage

Press Z as ETC Eos "Next" and C as "Last".

Use joystick to adjust Pan/Tilt on moving fixtures 

Hold down both Z & C for "fine" mode of movement

## Guide to Arduino LEDs

![](https://i.imgur.com/3if1ke8.png)
