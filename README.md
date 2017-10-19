# lighthack
This repository contains the source and documentation for lighthack, a new ETC project designed to engage with the DIY community. The lighthack project is centered around widgets that you assemble from scratch, which communicate via OSC with ETC's Eos software.

Current designs use the Arduino hardware/software platform to send and receive OSC to/from Eos using a USB COM port. Eos supports this functionality (on Windows 7 consoles and Nomad Windows) in version 2.6.1 and newer. Nomad Mac support is targeted for the near future. Windows XP console support is not planned for this project (but you can [upgrade your Windows XP console to Windows 7](http://www.etcconnect.com/Support/Articles/Windows-7-Upgrade-Eligibility.aspx) to make it compatible).

Each DIY widget has its own subfolder. This folder typically contains an assembly instructions document which is a good jumping-off point. It also contains the Arduino sketch and other supporting documentation.

[Supported OSC Commands](https://github.com/ETCLabs/EosSyncLib/blob/master/Supported%20OSC%20Commands.pdf)

## About this ETCLabs Project
The lighthack projects (developed by a combination of end users and ETC employees in their free time) are designed to interact with Electronic Theatre Controls products. This is not official ETC hardware or software. For challenges using, integrating, compiling, or modifying items in this project, we encourage posting on the [Issues](https://github.com/ElectronicTheatreControlsLabs/lighthack/issues) page. ETC Support is not familiar with this project and will not be able to assist if issues arise. (Hopefully issues won't happen, and you'll have a lot of fun with these tools and toys!)

# Dependencies
Requires the Arduino OSC parsing library maintained by the great folks over at [CNMAT](https://github.com/CNMAT/OSC). The assembly instructions documents contain details on how to add this to your Arduino project.
