//-----------------------------------------------------------------------------
//    Tiny Gaming LLC, Hanna Button Box Thingy
//    Copyright (C) 2022 Tiny Gaming LLC  - All Rights Reserved 
//    
//    This file is part of {Hanna Button Box Thingy}
//    Unauthorized copying of this file, via any medium is strictly
//    prohibited proprietary and confidential
//
//    {Tiny Gaming Hanna Button Box Thingy} can not be copied and/or distributed
//    without the express permission of {Tiny Gaming LLC}
//
//    Authors: McCawley Mark
//    Version 1.0.0
//
//    Functions:
//    void read_box_buttons(int polltime)
//    void read_box_switches(int polltime)
//    void read_matrix_buttons(int polltime)
//    void read_led_button(int polltime)
//    void draw_bitmap(int polltime)
//    void draw_text(String text, int x, int y,int size, boolean d, boolean c)
//    void setup()
//    void loop()
//
// Arduino Pro Micro wiring diagram
//
//                         +-----+
//                +--------| USB |-------+
//                |        +-----+       |
//                | [1] TX         [RAW] |
//                | [0] RX         [GND] |  Ground
//       Ground   | [GND]          [RST] |
//       Ground   | [GND]          [VCC] |  Voltage for OLED
//      OLED SDA  | [2] SDA      A3 [21] |  Small Button
//      OLED SCL  | [3] SCL      A2 [20] |  Large Button
//  Keypad Row 1  | [4] A6       A1 [19] |  L.E.D. Driver
//  Keypad Row 2  | [5]          A0 [18] |  L.E.D. Control
//  Keypad Row 3  | [6] A7     SCLK [15] |  Switch Four
//  Keypad Col 1  | [7]        MISO [14] |  Switch Three
//  Keypad Col 2  | [8] A8     MOSI [16] |  Switch Two
//  Keypad Col 3  | [9] A9      A10 [10] |  Switch One
//                |                      |
//                +----------------------+
//
//-----------------------------------------------------------------------------

#include <Joystick.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <Bounce2.h>
#include <EEPROM.h>
#include <Keypad.h>
#include <Arduino.h>

// Keypad rows and columns
const byte ROWS = 3; // Three rows
const byte COLS = 3; // Three columns
const long loop_interval = 10;  // Polling rate for main loop
const long button_interval = 1;   // Polling rate for box buttons
const long switch_interval = 1;   // Polling rate for box switches
const long keypad_interval = 1;   // Polling rate for box keypad
const bool initAutoSendState = true;  // Init send state always
const long brightness_interval = 0;    // Polling rate for LED brightness button
unsigned long display_counter;
const unsigned long period = 300;  // OLED refresh period

// Define the symbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] =
{
    {9, 8, 7},
    {12, 11, 10},
    {15, 14, 13},
};

// Init variables to track button states
int drake_button_1_last = 0;
int drake_button_2_last = 0;
int drake_switch_1_last = 0;
int drake_switch_2_last = 0;
int drake_switch_3_last = 0;
int drake_switch_4_last = 0;
int ledPin = 19; // LED connected to digital pin 19
int ledDuty = 0; // Variable to store the LED duty cycle
uint8_t rowPins[ROWS] = {9, 8, 7}; // Connect to the row pinouts of the keypad
uint8_t colPins[COLS] = {4, 5, 6}; // Connect to the column pinouts of the keypad
String switch_state[6] = { "OFF", "ON" };

// Define the OLED screen dimentions
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define LOGO_WIDTH 16
#define ledControl 18
#define keypad_row1 4
#define keypad_row2 5
#define keypad_row3 6
#define keypad_col1 7
#define keypad_col2 8
#define keypad_col3 9
#define LOGO_HEIGHT 16
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define drake_button_1 20
#define drake_button_2 21
#define drake_switch_1 15
#define drake_switch_2 14
#define drake_switch_3 16
#define drake_switch_4 10

// Initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// Initialize an instance of OLED screen
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Initialize an instance of Joystick for 15 buttons only.
// HID id, Type, Button Cnt, hat count, include X, Y, Z, Rx, Ry, Rz, Rudder, Throttle, Accellerator, Brake, Steering
Joystick_ Joystick(0x15, JOYSTICK_TYPE_JOYSTICK, 15, 0, false, false, false, false, false, false, false, false, false, false, false);

// Instantiate a Bounce object for the LED control button
Bounce led = Bounce();

/*!
 * @function    read_box_buttons
 * @abstract    Poll for box button change.
 * @discussion  Polls for box button states
 * @param       polling delay in milliseconds
 * @result      void
*/
void read_box_buttons(int polltime){
  // Start the polling for box button (1)
  int drake_button_1_current = !digitalRead(drake_button_1);
  if (drake_button_1_current != drake_button_1_last) {
    Joystick.setButton(9, drake_button_1_current);
    drake_button_1_last = drake_button_1_current;
    draw_text("BUTTON: 1", 3, 14, 2, true, true); // Draw text on the screen
  }
  // Start the polling for box button (2)
  int drake_button_2_current = !digitalRead(drake_button_2);
  if (drake_button_2_current != drake_button_2_last) {
    Joystick.setButton(10, drake_button_2_current);
    drake_button_2_last = drake_button_2_current;
    draw_text("BUTTON: 2", 3, 14, 2, true, true); // Draw text on the screen
  }
  delay(polltime);
}

/*!
 * @function    press_joystick_button
 * @abstract    Poll for box switch change.
 * @discussion  Sends single button press and release to joystick lib
 * @param       button Joystick button to press
 * @result      void
*/
void press_joystick_button(int button, int direction){
  Joystick.setButton(button, 1);
  delay(125);
  Joystick.setButton(button, 0);

  // Draw the text on the OLED screen
  draw_text("SW:", 8, 14, 2, false, true);
  draw_text(String(button - 10), 40, 14, 2, false, false);
  draw_text(switch_state[direction], 70, 14, 2, true, false);
}

/*!
 * @function    read_box_switch
 * @abstract    Poll for box switch change.
 * @discussion  Polls for box switch state change and sends single button press and release
 * @param       polling delay in milliseconds
 * @result      void
*/
void read_box_switch(int polltime){
  int drake_switch_1_current = !digitalRead(drake_switch_1);
  if (drake_switch_1_current != drake_switch_1_last) {
    press_joystick_button(11, drake_switch_1_current);
    drake_switch_1_last = drake_switch_1_current;
  }
  // Start the polling for box switch (2)
  int drake_switch_2_current = !digitalRead(drake_switch_2);
  if (drake_switch_2_current != drake_switch_2_last) {
    press_joystick_button(12, drake_switch_2_current);
    drake_switch_2_last = drake_switch_2_current;
  }
  // Start the polling for box switch (3)
  int drake_switch_3_current = !digitalRead(drake_switch_3);
  if (drake_switch_3_current != drake_switch_3_last) {
    press_joystick_button(13, drake_switch_3_current);
    drake_switch_3_last = drake_switch_3_current;
  }
  // Start the polling for box switch (4)
  int drake_switch_4_current = !digitalRead(drake_switch_4);
  if (drake_switch_4_current != drake_switch_4_last) {
    press_joystick_button(14, drake_switch_4_current);
    drake_switch_4_last = drake_switch_4_current;
  }
  delay(polltime);
}

/*!
 * @function    read_matrix_buttons
 * @abstract    Poll for matrix buttons change.
 * @discussion  Polls for matrix button states
 * @param       polling delay in milliseconds
 * @result      void
*/
void read_matrix_buttons(int polltime) {
  if (customKeypad.getKeys()) {
    for (int i = 0; i < LIST_MAX; i++) {
      if ( customKeypad.key[i].stateChanged ) {
        switch (customKeypad.key[i].kstate) {
          case PRESSED:
            Joystick.setButton(customKeypad.key[i].kcode, 1);
            break;
          case RELEASED:
            Joystick.setButton(customKeypad.key[i].kcode, 0);
            draw_text("KEYPAD:", 3, 14, 2, false, true); // Draw text on the screen
            draw_text(String(customKeypad.key[i].kcode), 95, 14, 2, true, false); // Draw text on the screen
            break;
          case IDLE:
          case HOLD:
          default:
            break;
        }
      }
    }
  }
  delay(polltime);
}

/*!
 * @function    read_led_button
 * @abstract    Poll for LED brightness change.
 * @discussion  Polls for LED brightness button press
 * @param       polling delay in milliseconds
 * @result      void
*/
void read_led_button(int polltime){
  // Poll for LED control button
  if ( led.fell() ) {
    ledDuty += 20;
    if ( ledDuty > 100 ) ledDuty = 0;
    analogWrite(ledPin, ledDuty); // analogRead values go from 0 to 1023, analogWrite values from 0 to 255
    EEPROM.write(0, ledDuty);  // Save the new LED duty cycle to EEPROM for next boot
  }
  delay(polltime);
}

/*
* draw_text(String text, int x, int y,int size, boolean d)
* text is the text string to be printed
* x is the integer x position of text
* y is the integer y position of text
* z is the text size, 1, 2, 3 etc
* d is either "true" or "false". Not sure, use true
*/
void draw_text(String text, int x, int y,int size, boolean d, boolean c) {
  if(c){
    display.clearDisplay();
  }
  // display.drawRect(1, 1, 126,31, WHITE);
  display.setTextSize(size);
  display.setTextColor(WHITE);
  display.setCursor(x,y);
  display.println(text);
  if(d){
    display.display();
  }
  display_counter = 0;
}

/*
 * @function    setup
 * @abstract    Inital setup method
 * @discussion  Define pin pullups and set variables
 * @param       
 * @result      none
*/
void setup() {

  Serial.begin(9600);

  // Set internal pullups on buttons
  pinMode(drake_button_1, INPUT_PULLUP);
  pinMode(drake_button_2, INPUT_PULLUP);
  pinMode(drake_switch_1, INPUT_PULLUP);
  pinMode(drake_switch_2, INPUT_PULLUP);
  pinMode(drake_switch_3, INPUT_PULLUP);
  pinMode(drake_switch_4, INPUT_PULLUP);
  pinMode(keypad_row1, INPUT_PULLUP);
  pinMode(keypad_row2, INPUT_PULLUP);
  pinMode(keypad_row3, INPUT_PULLUP);
  pinMode(keypad_col1, INPUT_PULLUP);
  pinMode(keypad_col2, INPUT_PULLUP);
  pinMode(keypad_col3, INPUT_PULLUP);

  pinMode(ledPin, OUTPUT);  // sets the pin as output
  led.attach(ledControl,INPUT_PULLUP); // Attach the debouncer to LED control button
  led.interval(25); // Use a debounce interval of 25 milliseconds

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // draw_bitmap();
  display_counter = 0;

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  display.setRotation(2); // Screen mounted upside down, so rotate 90 twice
  display.clearDisplay(); // Clear the buffer

  // Set current switch states to avoid OLED message on boot
  drake_switch_1_last = !digitalRead(drake_switch_1);
  drake_switch_2_last = !digitalRead(drake_switch_2);
  drake_switch_3_last = !digitalRead(drake_switch_3);
  drake_switch_4_last = !digitalRead(drake_switch_4);

  Joystick.begin(); // Start Joystick

  // Read stored LED brightness from EEPROM
  ledDuty = EEPROM.read(0);
  analogWrite(ledPin, ledDuty);
  delay(10);
  
  // display.drawRect(1, 1, 126,31, WHITE);
  draw_text("19897795", 14, 14, 2, true, true); // Draw text on the screen
  delay(2000);
  draw_text("HannahB888", 3, 12, 2, true, true); // Draw text on the screen
  delay(1000);
}

/*!
 * @function    loop
 * @abstract    Poll for change.
 * @discussion  Polls for changes in switch, pad,  and button state.
 * @param       
 * @result      none
*/
void loop() {
  read_box_buttons(button_interval); // poll for box button states
  read_box_switch(keypad_interval); // poll for box keypad states
  read_matrix_buttons(keypad_interval); // poll for box keypad states
  read_led_button(brightness_interval); // poll for backlight control

  if (display_counter >= period) {
    draw_text("DRAKE", 30, 14, 2, true, true); // Draw text on the screen
    display_counter = 0;
  }

  // Delay for loop_interval milliseconds for loop restart
  delay(loop_interval);
  ++display_counter;
}
