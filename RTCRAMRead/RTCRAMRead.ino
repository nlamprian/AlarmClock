/**
 * Description:
 * This example demonstrates how to interface the RAM of a DS1307 RTC module.
 * DS1307 RTC has a 56 Byte RAM. Its address space starts at 0x08 and ends at 0x3F.
 * This RAM space is excellent in cases where you want to store data safely
 * (that persist through power downs). You could use it in your project that
 * involves the RTC to avoid having to buy an SD shield and saving the those 
 * data on an SD card.
 * 
 * This example initially writes a string on the RTC's RAM and defines a variable 
 * COUNTER that is saved on the module's RAM too. Then, at every iteration of 
 * loop, it reads the variable COUNTER and the module's RAM altogether and then 
 * prints on the serial monitor the variable along with the string that is
 * contained in the module's RAM space.
 * 
 * Note:
 * The buffers of the Wire library, either for sending or receiving data, are
 * 32 bytes long. So, for example in the case of the DS1307, we can't read the 
 * whole RAM in one step. We'll have to read the first half first and the 
 * second half later.
 * 
 * Author:
 *   Nick Lamprianidis { paign10.ln [at] gmail [dot] com }
 * 
 * License:
 *   Copyright (c) 2014 Nick Lamprianidis 
 *   This code is released under the MIT license
 *   http://www.opensource.org/licenses/mit-license.php
 */

#include <Wire.h>

#define DS1307_ADDR 0x68  // DS1307's address

// We want to save a byte on the RTC's RAM
// This byte represents a variable and we have to give it
// a name and an address at which we want it to be saved
#define COUNTER_ADDR 0x3F

void setup()
{
	Serial.begin(9600);  // Initializes serial port
    // Waits for serial port to connect. Needed for Leonardo only
    while ( !Serial ) ;
    
	Wire.begin();  // Initializes Wire library
	
	String msg = " This is a string from DS1307_ADDR's RAM.";

	writeString2RTC(msg);  // Writes the String variable msg to the RTC's RAM
	writeByte(COUNTER_ADDR, 0);  // Initializes the variable COUNTER
}

void loop()
{
	uint8_t data;
	char ram[56];
	
	// Reads the data and prints them on the serial monitor
	data = readByte(COUNTER_ADDR);
	readRTCRAM(ram);
	Serial.print(data);
	Serial.println(ram);
	
	writeByte(COUNTER_ADDR, data + 1);  // Increments variable COUNTER
	delay(1000);
}


// Reads a byte from the DS1307's RAM at the provided address
// Returns the byte read from the module's RAM
uint8_t readByte(uint8_t address)
{
	Wire.beginTransmission(DS1307_ADDR);
	Wire.write(address);  // Sets the register pointer on the DS1307
	Wire.endTransmission(false);  // Performs the transmission and keeps control of the bus
	
	Wire.requestFrom(DS1307_ADDR, 1);  // Reads byte from the designated register
	return Wire.read();
}

// Writes a byte at the provided address on the DS1307's RAM
// Returns status of transmission. Read documentation of endTransmission,
// http://arduino.cc/en/Reference/WireEndTransmission
uint8_t writeByte(uint8_t address, uint8_t data)
{
	Wire.beginTransmission(DS1307_ADDR);
	Wire.write(address);  // Sets the register pointer on the DS1307
	Wire.write(data);  // Data to write at the designated register
	return Wire.endTransmission();  // Performs the transmission
}

// Writes a String variable at the beginning (0x08) of the RTC's RAM
// It sends the string in two steps. At first, it sends the 31 first
// characters and then the rest of the string. It null terminates 
// the string with a NULL character.
// The max size of the string is 54 characters.
void writeString2RTC(String& msg)
{
	uint8_t n = msg.length();
	
	Wire.beginTransmission(DS1307_ADDR);
	Wire.write(0x08);
	for (uint8_t i = 0; i < n && i < 31; ++i) Wire.write(msg[i]);
	if ( n < 31 ) Wire.write(0x0);
	Wire.endTransmission();
	
	if ( n > 31 )
	{
		Wire.beginTransmission(DS1307_ADDR);
		Wire.write(0x27);
		for (uint8_t i = 31; i < n && i < 54; ++i) Wire.write(msg[i]);
		Wire.write(0x0);
		Wire.endTransmission();
	}
}

// Reads the DS1307's entire RAM
// It reads it in 2 steps (28 first bytes + the rest 28 bytes)
// The data are saved in the provided variable
void readRTCRAM(char *ram)
{
	uint8_t i = 0;
	
	Wire.beginTransmission(DS1307_ADDR);
	Wire.write(0x08);
	Wire.endTransmission(false);
	Wire.requestFrom(DS1307_ADDR, 28, false);
	while (Wire.available()) ram[i++] = Wire.read();
	Wire.requestFrom(DS1307_ADDR, 28);
	while (Wire.available()) ram[i++] = Wire.read();
}
