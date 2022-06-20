# DRAKE
Drake button box

This is the code for the windows HID button box in the style of Star Citizen DRAKE design.
The original design was by HANNAHB888 you can find her original artwork for this box as well as other work she's done at https://www.reddit.com/r/starcitizen/comments/hph00n/drake_interplanetary_smartkey_thing_that_i_made/

This box was designed off of her original artwork, and turned into a functional model, and code.
You can find the 3D printable files over at Thingaverse at: https://www.thingiverse.com/thing:5414999

The box uses an Arduino pro micro, and while there is no wiring schematic, the following is quite straigt forward.
All pins are labled as to what they connect to.

The four switches connec to Arduino pins 15, 14, 16, 10, and the other end of the switch connects to ground.
Arduino pins 19 drives L.E.D's if you want to run backlit keys. Arduino pin 18, is a button to ground that changes brioghtness.
The brightness is written to EEPROM so it remains after USB disconnection.

Pins 2, and 3 drive the OLED screen.
Pins 4, 5, 6, 7, 8, 9, All drive the button matrix. See the links below for setting up a button matrix.
    https://haneefputtur.com/making-matrix-keypad-with-push-buttons-arduino.html
    https://github-wiki-see.page/m/SHWotever/SimHub/wiki/Arduino---Button-matrix
I'm sure there are more sites with info out there. A bit of reading and it's quite understandable


## Arduino Pro Micro wiring diagram

                            +-----+
                   +--------| USB |-------+
                   |        +-----+       |
                   | [1] TX         [RAW] |
                   | [0] RX         [GND] |  Ground
          Ground   | [GND]          [RST] |
          Ground   | [GND]          [VCC] |  Voltage for OLED
         OLED SDA  | [2] SDA      A3 [21] |  Small Button
         OLED SCL  | [3] SCL      A2 [20] |  Large Button
     Keypad Row 1  | [4] A6       A1 [19] |  L.E.D. Driver
     Keypad Row 2  | [5]          A0 [18] |  L.E.D. Control
     Keypad Row 3  | [6] A7     SCLK [15] |  Switch Four
     Keypad Col 1  | [7]        MISO [14] |  Switch Three
     Keypad Col 2  | [8] A8     MOSI [16] |  Switch Two
     Keypad Col 3  | [9] A9      A10 [10] |  Switch One
                   |                      |
                   +----------------------+
