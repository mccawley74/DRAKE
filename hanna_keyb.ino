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
//  Switch Five   | [1] TX         [RAW] |
//                | [0] RX         [GND] |  Ground
//       Ground   | [GND]          [RST] |
//       Ground   | [GND]          [VCC] |  Voltage for OLED
//      OLED SDA  | [2] SDA      A3 [21] |  Small Button
//      OLED SCL  | [3] SCL      A2 [20] |  Large Button
//  Keypad Row 1  | [4] A6       A1 [19] |  Switch One
//  Keypad Row 2  | [5]          A0 [18] |  L.E.D. Control input
//  Keypad Row 3  | [6] A7     SCLK [15] |  Switch Four
//  Keypad Col 1  | [7]        MISO [14] |  Switch Three
//  Keypad Col 2  | [8] A8     MOSI [16] |  Switch Two
//  Keypad Col 3  | [9] A9      A10 [10] |  L.E.D. PWM Vcc Out
//                |                      |
//                +----------------------+
//
//  NOTE: Switch One and Switch Five is a two way switch, center to ground
//  All switches are connected from the arduino GPIO to ground
//  
//
//-----------------------------------------------------------------------------

#include <Keyboard.h>
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
char keymap_1[ROWS][COLS] ={{0, 1, 2}, {3, 4, 5}, {6, 7, 8}};

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
#define drake_switch_4 19
#define drake_switch_5 1

// Instantiate a Bounce object for the LED control button
Bounce led = Bounce();

// Init variables to track button states
char key_pressed;
int led_buttonState = 0;
int drake_button_1_last = HIGH;
int drake_button_2_last = HIGH;
int drake_switch_1_last = 0;
int drake_switch_5_last = 0;
int drake_switch_2_last = 0;
int drake_switch_3_last = 0;
int drake_switch_4_last = 0;
int profile = 1;
int ledPin = 10; // LED connected to digital pin 10
int ledDuty = 0; // Variable to store the LED duty cycle
uint8_t rowPins[ROWS] = {9, 8, 7}; // Connect to the row pinouts of the keypad
uint8_t colPins[COLS] = {4, 5, 6}; // Connect to the column pinouts of the keypad

// Button mapping, change to match keyboard mapping in game
// https://www.arduino.cc/reference/en/language/functions/usb/keyboard/keyboardmodifiers/
char button_map[3] = {KEY_LEFT_ALT, 'Y', 'r'}; // Eject, Eject, Flight Ready
char switch_map[3] = {'l', 'k', 'n'}; // Light, VTOL, Landing gear
char key_map_one[9] = {KEY_F1, KEY_F2, KEY_F11, 'p', 'o', 'i', 'u', KEY_ESC, KEY_ESC};
char key_map_two[9] = {KEY_F5, KEY_ESC, KEY_F6, KEY_ESC, KEY_F8, KEY_ESC, KEY_ESC, KEY_F7, KEY_ESC}; // Power Triangle
char key_map_thr[9] = {0xE7, 0xE8, 0xE9, 0xE4, 0xE5, 0xE6, 0xE1, 0xE2, 0xE3}; // Shields | Maps numpad keys

// Initialize an instance of class NewKeypad
Keypad keypad_one = Keypad( makeKeymap(keymap_1), rowPins, colPins, ROWS, COLS);

// Initialize an instance of OLED screen
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/*!
 * @function    read_box_buttons
 * @abstract    Poll for box button change.
 * @discussion  Polls for box button states
 * @param       polling delay in milliseconds
 * @result      void
*/
void read_box_buttons(int polltime){
  int drake_button_1_current = digitalRead(drake_button_1);
  int drake_button_2_current = digitalRead(drake_button_2);
  
  if (drake_button_1_current == LOW && drake_button_1_last == HIGH) {
    Keyboard.press(button_map[2]);
    delay(50);
  }
  if (drake_button_1_current == HIGH && drake_button_1_last == LOW) {
    Keyboard.release(button_map[2]);
    delay(50);
    draw_text("BUTTON: 1", 3, 14, 2, true, true); // Draw text on the screen
  }
  
  if (drake_button_2_current == LOW && drake_button_2_last == HIGH) {
    Keyboard.press(button_map[0]);
    Keyboard.press(button_map[1]);
    delay(50);
  }
  if (drake_button_2_current == HIGH && drake_button_2_last == LOW) {
    Keyboard.release(button_map[1]);
    Keyboard.release(button_map[0]);
    delay(50);
    draw_text("BUTTON: 2", 3, 14, 2, true, true); // Draw text on the screen
  }

  drake_button_1_last = drake_button_1_current;
  drake_button_2_last = drake_button_2_current;
  delay(polltime);
}

/*!
 * @function    read_box_switch
 * @abstract    Poll for box switch change.
 * @discussion  Polls for box switch state change and sends single button press and release
 * @param       polling delay in milliseconds
 * @result      void
*/
void read_box_switch(int polltime){
  // Start the polling for box switch (1)
  int drake_switch_1_current = !digitalRead(drake_switch_1);
  if (drake_switch_1_current != drake_switch_1_last) {
    drake_switch_1_last = drake_switch_1_current;
    set_profile(drake_switch_1_last, drake_switch_5_last);
  }
  // Start the polling for box switch (5)
  int drake_switch_5_current = !digitalRead(drake_switch_5);
  if (drake_switch_5_current != drake_switch_5_last) {
    drake_switch_5_last = drake_switch_5_current;
    set_profile(drake_switch_1_last, drake_switch_5_last);
  }
  // Start the polling for box switch (2)
  int drake_switch_2_current = !digitalRead(drake_switch_2);
  if (drake_switch_2_current != drake_switch_2_last) {
    press_key(switch_map[0]);
    drake_switch_2_last = drake_switch_2_current;
  }
  // Start the polling for box switch (3)
  int drake_switch_3_current = !digitalRead(drake_switch_3);
  if (drake_switch_3_current != drake_switch_3_last) {
    press_key(switch_map[1]);
    drake_switch_3_last = drake_switch_3_current;
  }
  // Start the polling for box switch (4 Left)
  int drake_switch_4_current = !digitalRead(drake_switch_4);
  if (drake_switch_4_current != drake_switch_4_last) {
    press_key(switch_map[2]);
    drake_switch_4_last = drake_switch_4_current;
  }
  delay(polltime);
}

/*!
 * @function    Pressed a keyboard key
 * @abstract    press key
 * @discussion  press key
 * @param       key The key to press
 * @result      void
*/
void press_key(char key) {
  Keyboard.press(key);
  delay(50);
  Keyboard.release(key);
  delay(50);
}

/*!
 * @function    read_matrix_buttons
 * @abstract    Poll for matrix buttons change.
 * @discussion  Polls for matrix button states
 * @param       polling delay in milliseconds
 * @result      void
*/
void read_matrix_buttons(int polltime) {
  if (keypad_one.getKeys()) {
    for (int i = 0; i < LIST_MAX; i++) {
      if ( keypad_one.key[i].stateChanged ) {
        if (profile == 1) {
          key_pressed = key_map_one[keypad_one.key[i].kcode];
        } else if (profile == 2) {
          key_pressed = key_map_two[keypad_one.key[i].kcode];
        } else if (profile == 3) {
          key_pressed = key_map_thr[keypad_one.key[i].kcode];
        } else {
          key_pressed = key_map_one[keypad_one.key[i].kcode];
        }
        switch (keypad_one.key[i].kstate) {
          case PRESSED:
            Keyboard.press(key_pressed);
            break;
          case RELEASED:
            Keyboard.release(key_pressed);
            draw_text("KEYPAD:", 3, 14, 2, false, true); // Draw text on the screen
            draw_text(String(keypad_one.key[i].kcode), 95, 14, 2, true, false); // Draw text on the screen
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

  led_buttonState = digitalRead(ledControl);
  if (led_buttonState == HIGH) {
    // turn LED on:
  } else {
    // turn LED off:
    Serial.println("LOW");
    Serial.println(ledDuty);
    ledDuty += 1;
    if ( ledDuty > 100 ) ledDuty = 0;
    analogWrite(ledPin, ledDuty); // analogRead values go from 0 to 1023, analogWrite values from 0 to 255
  }
  
  if ( led.fell() ) {
    Serial.write("Read LED Pin"); // send a byte with the value 45
    ledDuty += 20;
    if ( ledDuty > 200 ) ledDuty = 0;
    analogWrite(ledPin, ledDuty); // analogRead values go from 0 to 1023, analogWrite values from 0 to 255
    EEPROM.write(0, ledDuty);  // Save the new LED duty cycle to EEPROM for next boot
    draw_text("LED PRESS:", 3, 14, 2, false, true); // Draw text on the screen
  }
  delay(polltime);
}

/*!
 * @function    set_profile
 * @abstract    Set the keypad mapping based on switch position.
 * @discussion  Switch mode. Switch one and five is a two way switch
 * @param       sw1 switch one state
 * @param       sw2 switch five state
 * @result      void
*/
void set_profile(int sw1, int sw2){
  if (sw1 == 1 && sw2 == 0) {
    profile = 1;
  } else if (sw1 == 0 && sw2 == 1) {
    profile = 3;
  } else if (sw1 == 0 && sw2 == 0) {
    profile = 2;
  }
  draw_text("PROFILE:", 3, 14, 2, false, true); // Draw text on the screen
  draw_text(String(profile), 97, 14, 2, true, false); // Draw text on the screen
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
  pinMode(drake_switch_5, INPUT_PULLUP);
  pinMode(keypad_row1, INPUT_PULLUP);
  pinMode(keypad_row2, INPUT_PULLUP);
  pinMode(keypad_row3, INPUT_PULLUP);
  pinMode(keypad_col1, INPUT_PULLUP);
  pinMode(keypad_col2, INPUT_PULLUP);
  pinMode(keypad_col3, INPUT_PULLUP);

  // Setup L.E.D. brightness controls
  pinMode(ledPin, OUTPUT);  // sets the pin as output
  led.attach(ledControl,INPUT_PULLUP); // Attach the debouncer to LED control button
  led.interval(25); // Use a debounce interval of 25 milliseconds

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Setup initial  display
  display_counter = 0;
  display.display();
  display.setRotation(2); // Screen mounted upside down, so rotate 90 twice
  display.clearDisplay(); // Clear the buffer

  // Set current switch states to avoid OLED message on boot
  drake_switch_1_last = !digitalRead(drake_switch_1);
  drake_switch_2_last = !digitalRead(drake_switch_2);
  drake_switch_3_last = !digitalRead(drake_switch_3);
  drake_switch_4_last = !digitalRead(drake_switch_4);
  drake_switch_5_last = !digitalRead(drake_switch_5);

  // Initialize the keyboard library
  Keyboard.begin();

  // Read stored LED brightness from EEPROM
  ledDuty = EEPROM.read(0);
  analogWrite(ledPin, ledDuty);
  delay(50);
  
  // display.drawRect(1, 1, 126,31, WHITE);
  draw_text("DRAKE", 30, 14, 2, true, true); // Draw text on the screen
  delay(150);
  set_profile(!digitalRead(drake_switch_1), !digitalRead(drake_switch_5));
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
