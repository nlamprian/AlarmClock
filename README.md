AlarmClock
==========

This is an example Arduino sketch that builds an Alarm Clock out of an Arduino Uno, an [`LCD Keypad Shield`](http://www.sainsmart.com/sainsmart-1602-lcd-keypad-shield-for-arduino-duemilanove-uno-mega2560-mega1280.html) (the one with the parallel interface, not the I2C), a [`DS1307 Real Time Clock`](https://www.sparkfun.com/products/99), and a buzzer.

![project](https://github.com/nlamprian/AlarmClock/wiki/assets/project.png)

The example can basically display `date` and `time`, set an `alarm`, ring a `buzzer` when the time comes, and `snooze`.

The code was implemented based on the following `FSM` design. For details on the operation of the clock/alarm, please read the description in the example code.

![fsm](https://github.com/nlamprian/AlarmClock/wiki/assets/fsm.png)

Dependencies
------------

The code makes use of the [`Jeelab RTClib`](https://github.com/adafruit/RTClib) and [`OSEPP LCDKeypad`](http://osepp.com/products/shield-arduino-compatible/16x2-lcd-display-keypad-shield/) libraries.
