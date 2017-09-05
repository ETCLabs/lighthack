# USB Test Sketch
This folder contains a troubleshooting tool to verify the line of communication between an Arduino and the Eos software. Once connected with Eos, it sends a series of OSC "ping" messages which are visible in the Eos diagnostics tab. Follow these steps to use it:

1. Burn the sketch to your Arduino using the Arduino IDE.
2. Start the Eos Family software, and connect your device to the machine running Eos (if not the same machine).
3. Navigate to the Eos diagnostics tab ([Tab & 99]). You should see log messages about an OSC handshake completing:

![handshake messages](https://github.com/ETCLabs/OSCHardware/raw/master/test_tools/usb_test/handshake.png)

4. On the diagnostics tab, click the button labeled "Incoming OSC (off)" to turn on display of OSC messages. You should see a ping message coming from your device once per second with an incrementing number argument:

![ping messages](https://github.com/ETCLabs/OSCHardware/raw/master/test_tools/usb_test/ping.png)

If this doesn't work, there is a non-hardware-related problem. Make sure the Arduino USB drivers are installed on your target machine. Check the Device Manager to make sure the Arduino is being recognized correctly.
