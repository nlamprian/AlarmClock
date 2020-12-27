/**
 * Description:
 * This is an example sketch that builds an Alarm Clock out of an Arduino Uno,
 * an LCD Keypad Shield, a DS1307 Real Time Clock, and a buzzer
 * The example lets you display the date and time, and also set the alarm.
 * There are 4 modes of operation in which the buttons on the LCD have specific functions.
 * "Display Date Time" mode: Date, Time, and an Alarm indication display on the LCD (default mode)
 *      SELECT: Lets you set the alarm time
 *      LEFT: Displays the current alarm time
 *      RIGHT: Turns the alarm on and off
 * "Display Alarm Time": 
 *      No active buttons. After 3 seconds the clock returns to the default mode
 * "Set Alarm" mode: Lets you set the alarm time (hours & minutes)
 *      UP: Increases hours/minutes **
 *      DOWN: Decreases hours/minutes **
 *      SELECT: Accepts the hours/minutes value
 * "Buzzer Ringing" mode: When the alarm time has been met, the buzzer rings
 *      SELECT or LEFT: Turns off the buzzer and the alarm
 *      UP or DOWN: Snooze -> Adds 10 minutes to the alarm time
 * ** After 5 seconds of inactivity, the clock returns to default mode
 * 
 * Note 1:
 * The development of the code was based on a Finite State Machine (FSM)
 * you can find on github: https://github.com/nlamprian/AlarmClock
 * The code is quite simple and was made possible due to the above 
 * preliminary design. You are urged to study both the FSM and the code.
 * Note 2:
 * The time on the RTC can shift over time. If you notice a deviation building up,
 * just uncomment line 91 and load the example to your Arduino. That will set the
 * computer's time on the RTC. Afterwards, make sure to reupload the code with
 * line 91 commented out. If you don't do that, the next time your Arduino resets
 * it will write the time again on the RTC... the time of the code's compilation.
 * 
 * Author:
 *   Nick Lamprianidis { nlamprian [at] gmail [dot] com }
 * 
 * License:
 *   Copyright (c) 2014 Nick Lamprianidis 
 *   This code is released under the MIT license
 *   http://www.opensource.org/licenses/mit-license.php
 */

#include <Wire.h>  // Required by RTClib
#include <LiquidCrystal.h>  // Required by LCDKeypad
#include <LCDKeypad.h>
#include "RTClib.h"

#define TIME_OUT 5  // One of the system's FSM transitions
#define ALARM_TIME_MET 6  // One of the system's FSM transitions

#define BUZZER_PIN 3  // Output PWM pin for the buzzer
#define SNOOZE 10  // Minutes to snooze

// The different states of the system
enum states
{
    SHOW_TIME,           // Displays the time and date
    SHOW_TIME_ALARM_ON,  // Displays the time and date, and alarm is on
    SHOW_ALARM_TIME,     // Displays the alarm time and goes back to time and date after 3 seconds
    SET_ALARM_HOUR,      // Option for setting the alarm hours. If provided, it moves on to alarm minutes.
                         //   Otherwise, it times out after 5 seconds and returns to time and date
    SET_ALARM_MINUTES,   // Option for setting the alarm minutes. If provided, it finally sets the alarm time and alarm.
                         //   Otherwise, it times out after 5 seconds and returns to time and date
    BUZZER_ON            // Displays the time and date, and buzzer is on (alarm time met)
};

// Creates an LCDKeypad instance
// It handles the LCD screen and buttons on the shield
LCDKeypad lcd;

// Creates an RTC_DS1307 instance
// It handles the DS1307 Real-Time Clock
RTC_DS1307 RTC;

states state;  // Holds the current state of the system
int8_t button;  // Holds the current button pressed
uint8_t alarmHours = 0, alarmMinutes = 0;  // Holds the current alarm time
uint8_t tmpHours;
boolean alarm = false;  // Holds the current state of the alarm
unsigned long timeRef;
DateTime now;  // Holds the current date and time information


void setup()
{
    pinMode(BUZZER_PIN, OUTPUT);  // Buzzer pin

    // Initializes the LCD and RTC instances
    lcd.begin(16, 2);
    Wire.begin();
    RTC.begin();

    state = SHOW_TIME;  // Initial state of the FSM
 
    // Uncomment this to set the current time on the RTC module
    // RTC.adjust(DateTime(__DATE__, __TIME__));
}

// Has the main control of the FSM (1Hz refresh rate)
void loop()
{
    timeRef = millis();

    // Uses the current state to decide what to process
    switch (state)
    {
        case SHOW_TIME:
            showTime();
            break;
        case SHOW_TIME_ALARM_ON:
            showTime();
            checkAlarmTime();
            break;
        case SHOW_ALARM_TIME:
            showAlarmTime();
            break;
        case SET_ALARM_HOUR:
            setAlarmHours();
            if ( state != SET_ALARM_MINUTES ) break;
        case SET_ALARM_MINUTES:
            setAlarmMinutes();
            break;
        case BUZZER_ON:
            showTime();
            break;
    }

    // Waits about 1 sec for events (button presses)
    // If a button is pressed, it blocks until the button is released
    // and then it performs the applicable state transition
    while ( (unsigned long)(millis() - timeRef) < 970 )
    {
        if ( (button = lcd.button()) != KEYPAD_NONE )
        {
            while ( lcd.button() != KEYPAD_NONE ) ;
            transition(button);
            break;
        }
    }
}

// Looks at the provided trigger (event) 
// and performs the appropriate state transition
// If necessary, sets secondary variables
void transition(uint8_t trigger)
{
    switch (state)
    {
        case SHOW_TIME:
            if ( trigger == KEYPAD_LEFT ) state = SHOW_ALARM_TIME;
            else if ( trigger == KEYPAD_RIGHT ) { alarm = true; state = SHOW_TIME_ALARM_ON; }
            else if ( trigger == KEYPAD_SELECT ) state = SET_ALARM_HOUR;
            break;
        case SHOW_TIME_ALARM_ON:
            if ( trigger == KEYPAD_LEFT ) state = SHOW_ALARM_TIME;
            else if ( trigger == KEYPAD_RIGHT ) { alarm = false; state = SHOW_TIME; }
            else if ( trigger == KEYPAD_SELECT ) state = SET_ALARM_HOUR;
            else if ( trigger == ALARM_TIME_MET ) { analogWrite(BUZZER_PIN, 220); state = BUZZER_ON; }
            break;
        case SHOW_ALARM_TIME:
            if ( trigger == TIME_OUT ) { if ( !alarm ) state = SHOW_TIME;
                                         else state = SHOW_TIME_ALARM_ON; }
            break;
        case SET_ALARM_HOUR:
            if ( trigger == KEYPAD_SELECT ) state = SET_ALARM_MINUTES;
            else if ( trigger == TIME_OUT ) { if ( !alarm ) state = SHOW_TIME;
                                              else state = SHOW_TIME_ALARM_ON; }
            break;
        case SET_ALARM_MINUTES:
            if ( trigger == KEYPAD_SELECT ) { alarm = true; state = SHOW_TIME_ALARM_ON; }
            else if ( trigger == TIME_OUT ) { if ( !alarm ) state = SHOW_TIME;
                                              else state = SHOW_TIME_ALARM_ON; }
            break;
        case BUZZER_ON:
            if ( trigger == KEYPAD_UP || trigger == KEYPAD_DOWN ) { analogWrite(BUZZER_PIN, 0); 
                                                                    snooze(); state = SHOW_TIME_ALARM_ON; }
            if ( trigger == KEYPAD_SELECT || trigger == KEYPAD_LEFT ) { analogWrite(BUZZER_PIN, 0); 
                                                                       alarm = false; state = SHOW_TIME; }
            break;
    }
}

// Displays the current date and time, and also an alarm indication
// e.g. SAT 04 JAN 2014, 22:59:10  ALARM
void showTime()
{
    now = RTC.now();
    const char* dayName[] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
    const char* monthName[] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
    
    lcd.clear();
    lcd.print(String(dayName[now.dayOfTheWeek()]) + " " +
              (now.day() < 10 ? "0" : "") + now.day() + " " +
              monthName[now.month()-1] + " " + now.year());
    lcd.setCursor(0,1);
    lcd.print((now.hour() < 10 ? "0" : "") + String(now.hour()) + ":" +
              (now.minute() < 10 ? "0" : "") + now.minute() + ":" +
              (now.second() < 10 ? "0" : "") + now.second() + (alarm ? "  ALARM" : ""));
}

// Displays the current alarm time and transitions back to show 
// date and time after 2 sec (+ 1 sec delay from inside the loop function)
// e.g. Alarm Time HOUR: 08 MIN: 20
void showAlarmTime()
{
    lcd.clear();
    lcd.print("Alarm Time");
    lcd.setCursor(0,1);
    lcd.print(String("HOUR: ") + ( alarmHours < 9 ? "0" : "" ) + alarmHours + 
                   " MIN: " + ( alarmMinutes < 9 ? "0" : "" ) + alarmMinutes);
    delay(2000);
    transition(TIME_OUT);
}

// Checks if the alarm time has been met, 
// and if so initiates a state transition
void checkAlarmTime()
{
    if ( now.hour() == alarmHours && now.minute() == alarmMinutes ) transition(ALARM_TIME_MET);
}

// When the buzzer is ringing, by pressing the UP or DOWN buttons, 
// a SNOOZE (default is 10) minutes delay on the alarm time happens
void snooze()
{
    alarmMinutes += SNOOZE;
    if ( alarmMinutes > 59 )
    {
        alarmHours += alarmMinutes / 60;
        alarmMinutes = alarmMinutes % 60;
    }
}

// The first of a 2 part process for setting the alarm time
// Receives the alarm time hour. If not provided within 5 sec,
// times out and returns to a previous (time and date) state
void setAlarmHours()
{
    unsigned long timeRef;
    boolean timeOut = true;
    
    lcd.clear();
    lcd.print("Alarm Time");

    tmpHours = 0;
    timeRef = millis();
    lcd.setCursor(0,1);
    lcd.print("Set hours:  0");
    
    while ( (unsigned long)(millis() - timeRef) < 5000 )
    {
        uint8_t button = lcd.button();

        if ( button == KEYPAD_UP )
        {
            tmpHours = tmpHours < 23 ? tmpHours + 1 : tmpHours;
            lcd.setCursor(11,1);
            lcd.print("  ");
            lcd.setCursor(11,1);
            if ( tmpHours < 10 ) lcd.print(" ");
            lcd.print(tmpHours);
            timeRef = millis();
        }
        else if ( button == KEYPAD_DOWN )
        {
            tmpHours = tmpHours > 0 ? tmpHours - 1 : tmpHours;
            lcd.setCursor(11,1);
            lcd.print("  ");
            lcd.setCursor(11,1);
            if ( tmpHours < 10 ) lcd.print(" ");
            lcd.print(tmpHours);
            timeRef = millis();
        }
        else if ( button == KEYPAD_SELECT )
        {
            while ( lcd.button() != KEYPAD_NONE ) ;
            timeOut = false;
            break;
        }
        delay(150);
    }

    if ( !timeOut ) transition(KEYPAD_SELECT);
    else transition(TIME_OUT);
}

// The second of a 2 part process for setting the alarm time
// Receives the alarm time minutes. If not provided within 5 sec,
// times out and returns to a previous (time and date) state
// If minutes are provided, sets the alarm time and turns the alarm on
void setAlarmMinutes()
{
    unsigned long timeRef;
    boolean timeOut = true;
    uint8_t tmpMinutes = 0;
    
    lcd.clear();
    lcd.print("Alarm Time");

    timeRef = millis();
    lcd.setCursor(0,1);
    lcd.print("Set minutes:  0");
    
    while ( (unsigned long)(millis() - timeRef) < 5000 )
    {
        uint8_t button = lcd.button();
        
        if ( button == KEYPAD_UP )
        {
            tmpMinutes = tmpMinutes < 55 ? tmpMinutes + 5 : tmpMinutes;
            lcd.setCursor(13,1);
            lcd.print("  ");
            lcd.setCursor(13,1);
            if ( tmpMinutes < 10 ) lcd.print(" ");
            lcd.print(tmpMinutes);
            timeRef = millis();
        }
        else if ( button == KEYPAD_DOWN )
        {
            tmpMinutes = tmpMinutes > 0 ? tmpMinutes - 5 : tmpMinutes;
            lcd.setCursor(13,1);
            lcd.print("  ");
            lcd.setCursor(13,1);
            if ( tmpMinutes < 10 ) lcd.print(" ");
            lcd.print(tmpMinutes);
            timeRef = millis();
        }
        else if ( button == KEYPAD_SELECT )
        {
            while ( lcd.button() != KEYPAD_NONE ) ;
            timeOut = false;
            break;
        }
        delay(150);
    }

    if ( !timeOut )
    {
        alarmHours = tmpHours;
        alarmMinutes = tmpMinutes;
        transition(KEYPAD_SELECT);
    }
    else transition(TIME_OUT);
}
