# Womier K66 v1


https://user-images.githubusercontent.com/5511687/156899040-eb86424e-6cbe-459c-ba1b-005fee8d5d4c.mp4


A compact 60% leyboard for RGB freaks with 66 keys/LEDs and 43 underglow LEDs in a acrylic housing.
Brought to you by: https://github.com/SonixQMK/qmk_firmware

Currently only hardware version 1 is supported.It has an VS11K16A MCU and VS12L03A I2C LED driver.\
Version 1 can be distinguished by the size of the chips on the backside of the board. There should be one square (MCU) and one rectangular (LED) chip.

Make example for this keyboard (after setting up your build environment):\
`make womier/k66:default`\
or\
`qmk compile -kb womier/k66 -km default`

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

KLE Layout here: [http://www.keyboard-layout-editor.com](http://www.keyboard-layout-editor.com/##@_name=Womier%20K66%20v1&author=fightforlife%3B&@_a:3%3B&=Esc%0A%0A%0A%0A%60%0A%0A%0A%0A%0A%0A%0A~&_a:7%3B&=1%20!%0A%0A%0A%0AF1&=2%20%2F@%0A%0A%0A%0AF2&=3%20%23%0A%0A%0A%0AF3&=4%20$%0A%0A%0A%0AF4&=5%20%25%0A%0A%0A%0AF5&=6%20%5E%0A%0A%0A%0AF6&=7%20%2F&%0A%0A%0A%0AF7&=8%20*%0A%0A%0A%0AF8&=9%20(%0A%0A%0A%0AF9&=0%20)%0A%0A%0A%0AF10&=-%20%2F_%0A%0A%0A%0AF11&=%2F=%20+%0A%0A%0A%0AF12&_w:2%3B&=Backspace&_x:0.25%3B&=Insert%0A%0A%0A%0A+%3B&@_w:1.5%3B&=Tab&=Q&=W&=E&=R&=T&=Y&=U&=I&=O&=P&=%5B%20%7B&=%5D%20%7D&_w:1.5%3B&=%5C%20%20%7C&_x:0.25%3B&=Delete%0A%0A%0A%0A-%3B&@_w:1.75%3B&=Capslock&=A&=S&=D&=F&=g&=H&=J&=K&=L&=%2F%3B%20%2F:&='%20%22&_w:2.25%3B&=Enter%3B&@_w:2.25%3B&=Shift&=Z&=X&=C&=V&=B&=N&=M&=,%20%3C&=.%20%3E&=%2F%2F%20%3F&_w:2%3B&=Shift&=%E2%86%91%0A%0A%0A%0APgUp%3B&@_w:1.25%3B&=Ctrl%0A%0A%0A%0Ai&=Win&_w:1.25%3B&=Alt%0A%0A%0A%0Ai&_w:6.25%3B&=&_w:1.25%3B&=Alt&=Fn&_w:1.25%3B&=Ctrl%0A%0A%0A%0Ai&=%E2%86%90%0A%0A%0A%0A+&=%E2%86%93%0A%0A%0A%0APgDo&=%E2%86%92%0A%0A%0A%0A-)

### How to get QMK working on the Womier K66 v1:
1. Check if you have the correct version 1 of the board by checking the chips on the underside of the board
2. Download the Sonix Flasher from here: https://github.com/SonixQMK/sonix-flasher
3. Start the flasher and click "Reboot to Bootloader". Wait for the board to be in the Bootloader.
4.  Flash the correct jumploader for the womier K66. You can get a precompiled binary here: https://github.com/SonixQMK/sonix-keyboard-bootloader
5.  When the board is in Jumploader flash QMK (which you built before using the command above)

### How to get back to stock firmware
1. Download the Womier K66 Stock fimrware from here: https://github.com/SonixQMK/Mechanical-Keyboard-Database
2. Put the board into the Jumploader by holding Backspace while plugging it in.
3. Use the Sonix Flasher "Revert to Stock Firmware" button and flash the stock firmware


## Board, Chip, LED informations
<details>
  <summary>More info</summary>

example circuit diagram
https://www.circuit-diagram.org/circuits/544944b664534461bac3e757aab00fd8
![544944b664534461bac3e757aab00fd8](https://user-images.githubusercontent.com/5511687/156899330-f528e468-ec4c-4179-8ca8-3bb4632ecffd.svg)

## VS11K16A / SN32F268F MCU
### Pinning

| PIN | P (A0-D8) | CT16B PWM           | LED matrix                    | KEY matrix                  | Comment / Other                           |
| --- | --------- | ------------------- | ----------------------------- | --------------------------- | ----------------------------------------- |
| 1   | P2.0      |                     |                               |                             | not connected                             |
| 2   | P2.1      |                     |                               |                             | not connected                             |
| 3   | P2.2      |                     |                               |                             | not connected                             |
| 4   | P0.0      | CT16B1\_PWM0/CLKOUT | green sink, row 0, Top        |                             | via 2x restistors and npn J3Y             |
| 5   | P0.1      | CT16B1\_PWM1        | blue sink, row 0, Top         |                             | via 2x restistors and npn J3Y             |
| 6   | P0.2      | CT16B1\_PWM2        | red sink, row 0, Top          |                             | via 2x restistors and npn J3Y             |
| 7   | P0.3      | CT16B1\_PWM3        | green sink, row 1             |                             | via 2x restistors and npn J3Y             |
| 8   | P0.4      | CT16B1\_PWM4/SCL0   | blue sink, row 1              |                             | via 2x restistors and npn J3Y             |
| 9   | P0.5      | CT16B1\_PWM5/SDA0   | red sink, row 1               |                             | via 2x restistors and npn J3Y             |
| 10  | P0.6      | CT16B1\_PWM6        | green sink, row 2             |                             | via 2x restistors and npn J3Y, incl INS   |
| 11  | P0.7      | CT16B1\_PWM7        | blue sink, row 2              |                             | via 2x restistors and npn J3Y, incl INS   |
| 12  | P0.8      | CT16B1\_PWM8        | red ink, row 2                |                             | via 2x restistors and npn J3Y, incl INS   |
| 13  | P0.9      | CT16B1\_PWM9        | green sink, row 3             |                             | via 2x restistors and npn J3Y, incl DEL   |
| 14  | P0.10     | CT16B1\_PWM10       | blue sink, row 3              |                             | via 2x restistors and npn J3Y, incl DEL   |
| 15  | P0.11     | CT16B1\_PWM11       | red sink, row 3               |                             | via 2x restistors and npn J3Y, incl DEL   |
| 16  | P0.12     | CT16B1\_PWM12       | green sink, row 4, bottom     |                             | via 2x restistors and npn J3Y, incl RIGHT |
| 17  | P0.13     | CT16B1\_PWM13       | blue sink, row 4, bottom      |                             | via 2x restistors and npn J3Y, incl RIGHT |
| 18  | P0.14     | CT16B1\_PWM14       | red sink, row 4, bottom       |                             | via 2x restistors and npn J3Y, incl RIGHT |
| 19  | P0.15     | CT16B1\_PWM15       |                               |                             | I2C LED Driver SDA, PIN 19                |
| 20  | P3.0      | CT16B1\_PWM16       |                               |                             | I2C LED Driver SCL, PIN 20                |
| 21  | P3.1      | CT16B1\_PWM17       | col 0 source, via 2TY, left   | col 0, via T4 diode, left   | ESC, TAB, CAPSLOCK, SHIFT,CTRL            |
| 22  | P3.2      | CT16B1\_PWM18       | col 1 source, via 2TY         | col 1, via T4 diode         | 1, Q, A, Z, WIN                           |
| 23  | P3.3      | CT16B1\_PWM19       | col 2 source, via 2TY         | col 2, via T4 diode         | 2,W,S,X,ALT                               |
| 24  | P3.4      | CT16B1\_PWM20       | col 3 source, via 2TY         | col 3, via T4 diode         | 3,E,D,C                                   |
| 25  | P3.5      | CT16B1\_PWM21       | col 4 source, via 2TY         | col 4, via T4 diode         | 4,R,F,V                                   |
| 26  | P3.6      | SWCLK               | col 5 source, via 2TY         | col 5, via T4 diode         | 5,T,G,B,SPACE                             |
| 27  | P3.7      | SWDIO               | col 6 source, via 2TY         | col 6, via T4 diode         | 6,Y,H,N                                   |
| 28  | P3.8      | CT16B1\_PWM22       | col 7 source, via 2TY         | col 7, via T4 diode         | 7,U,J,M                                   |
| 29  | P2.3      |                     | col 8 source, via 2TY         | col 8, via T4 diode         | 8,I,K,COMMA,ALT                           |
| 30  | P2.4      |                     | col 9 source, via 2TY         | col 9, via T4 diode         | 9,O,L,K,DOT,FN                            |
| 31  | P2.5      |                     | col 10 source, via 2TY        | col 10, via T4 diode        | 0,P,SEMICOLON,SLASH,CTRL                  |
| 32  | P2.6      |                     | col 11 source, via 2TY        | col 11, via T4 diode        | MINUS,BRACKETOPEN,QUOTE,SHIFT, LEFT,      |
| 33  | P2.7      |                     | col 12 source, via 2TY        | col 12, via T4 diode        | PLUS,BACKETCLOSE,ENTER,UP,DOWN            |
| 34  | P2.8      |                     | col 13 source, via 2TY, right | col 13, via T4 diode, right | INS,DEL,RIGHT,BACKSPACE,BACKSLASH         |
| 35  | P2.9      |                     |                               |                             | LED Driver PIN 3 I2C\_SDB                 |
| 36  | P2.10     |                     |                               |                             | not connected                             |
| 37  | P1.5      | MOSI0               |                               | row 4, bottom               |                                           |
| 38  | P1.4      | MISO0               |                               | row 3 (incl. DEL)           |                                           |
| 39  | P1.3      | SCK0                |                               | row 2 (incl. INS)           |                                           |
| 40  | P1.2      | SEL0                |                               | row 1                       |                                           |
| 41  | P1.1      | CT16B0\_CAP0        |                               | row 0, top                  |                                           |
| 42  | P1.0      |                     |                               |                             | not connected                             |
| 43  |           | VDDIO1              |                               |                             | 5V USB                                    |
| 44  |           | VDD                 |                               |                             | 5V USB                                    |
| 45  |           | VREG33              |                               |                             |                                           |
| 46  |           | DP/PSCLK            |                               |                             | USB Data                                  |
| 47  |           | DN/PSDATA           |                               |                             | USB Data                                  |
| 48  |           | VSS                 |                               |                             | Ground                                    |  

### RGB Matrix KEY LEDs
PCB backside, USB port top left
| Matrix LED |  |
| ---------- | ---------------------- |
| left up    | blue, sink             |
| left down  | red, sink              |
| right up   | green sink             |
| right down | Source, positive       |


## VS12L03A / SLED1734X I2C LED Driver
### Pinning
| PIN | function  | LED sinks (1-43)     | LED source (1-43) | comment / other                |
| --- | --------- | -------------------- | ----------------- | ------------------------------ |
| 1   |           | LED 17-29 green      | LED 12, 30, 42    |                                |
| 2   | VDD       |                      |                   | 5V USB                         |
| 3   | SDB       |                      |                   | not connected                  |
| 4   | SYNC      |                      |                   | not connected                  |
| 5   | GND       |                      |                   | Ground USB                     |
| 6   | CS/R\_EXT |                      |                   | not connected, empty solderpad |
| 7   |           |                      | LED 11, 41        |                                |
| 8   |           |                      | LED 10, 23, 40    |                                |
| 9   |           |                      | LED 9, 22, 39     |                                |
| 10  |           |                      | LED 8, 21, 38     |                                |
| 11  |           |                      | LED 7, 20, 33, 37 |                                |
| 12  |           |                      | LED 6, 19, 32, 36 |                                |
| 13  |           |                      | LED 5, 18, 31, 35 |                                |
| 14  |           |                      | LED 4, 17, 34     |                                |
| 15  |           |                      |                   | not connected                  |
| 16  | C\_FILT   |                      |                   | not connected, empty solderpad |
| 17  | AGCIN     |                      |                   | not connected                  |
| 18  | MISO/AD   |                      |                   | to ground via resistor         |
| 19  | MOSI/SDA  |                      |                   | CT16B1\_PWM15 / P0.15          |
| 20  | SCK/SCL   |                      |                   | CT16B1\_PWM16 / P3.0           |
| 21  |           | LED 1-3, 34-43 red   | LED 16,29         |                                |
| 22  |           | LED 1-3, 34-43 blue  | LED 15, 28        |                                |
| 23  |           | LED 1-3, 34-43 green | LED 14, 27        |                                |
| 24  |           | LED 4-16 red         | LED 26            |                                |
| 25  |           | LED 4-16 blue        | LED 3, 25         |                                |
| 26  |           | LED 4-16 green       | LED 2, 24         |                                |
| 27  |           | LED 17-29 red        | LED 1             |                                |
| 28  |           | LED 17-29 blue       | LED 13, 43        |


### LED RAM map type 3
LEDs counted clockwise (switch side up), starting at the USB port (1-43)
|     | Frame 1 |     |     |     |     |     |     |     | Frame 2 |     |     |     |     |     |     |     |
| --- | ------- | --- | --- | --- | --- | --- | --- | --- | ------- | --- | --- | --- | --- | --- | --- | --- |
|     | A       | B   | C   | D   | E   | F   | G   | H   | I       | J   | K   | L   | M   | N   | O   | P   |
| CA1 |         |     |     | R3  | R2  | R1  | R43 | R42 | R41     | R40 | R39 | R38 | R37 | R36 | R35 | R34 |
| CA2 |         |     |     | B3  | B2  | B1  | B43 | B42 | B41     | B40 | B39 | B38 | B37 | B36 | B35 | B34 |
| CA3 |         |     |     | G3  | G2  | G1  | G43 | G42 | G41     | G40 | G39 | G38 | G37 | G36 | G35 | G34 |
| CA4 | R16     | R15 | R14 |     |     |     | R13 | R12 | R11     | R10 | R9  | R8  | R7  | R6  | R5  | R4  |
| CA5 | B16     | B15 | B14 |     |     |     | B13 | B12 | B11     | B10 | B9  | B8  | B7  | B6  | B5  | B4  |
| CA6 | G16     | G15 | G14 |     |     |     | G13 | G12 | G11     | G10 | G9  | G8  | G7  | G6  | G5  | G4  |
| CA7 | R29     | R28 | R27 | R26 | R25 | R24 |     |     |         | R23 | R22 | R21 | R20 | R19 | R18 | R17 |
| CA8 | B29     | B28 | B27 | B26 | B25 | B24 |     |     |         | B23 | B22 | B21 | B20 | B19 | B18 | B17 |
| CA9 | G29     | G28 | G27 | G26 | G25 | G24 |     |     |         | G23 | G22 | G21 | G20 | G19 | G18 | G17 |
| CB1 |         |     |     |     |     |     |     |     | R30     |     |     |     | R33 | R32 | R31 |     |
| CB2 |         |     |     |     |     |     |     |     | B30     |     |     |     | B33 | B32 | B31 |     |
| CB3 |         |     |     |     |     |     |     |     | G30     |     |     |     | G33 | G32 | G31 |     |
| CB4 |         |     |     |     |     |     |     |     |         |     |     |     |     |     |     |     |
| CB5 |         |     |     |     |     |     |     |     |         |     |     |     |     |     |     |     |
| CB6 |         |     |     |     |     |     |     |     |         |     |     |     |     |     |     |     |
| CB7 |         |     |     |     |     |     |     |     |         |     |     |     |     |     |     |     |
| CB8 |         |     |     |     |     |     |     |     |         |     |     |     |     |     |     |

### Underglow LEDs
Pins from left to right with the light pointing up, pins down.
LEDs counted clockwise (switch side up), starting at the USB port (1-43)

| PIN          |  |
| ------------ | ----------------------------------------- |
| left         | red sink                                  |
| left middle  | source                                    |
| right middle | blue sink                                 |
| right        | green sink                                |


</details>









