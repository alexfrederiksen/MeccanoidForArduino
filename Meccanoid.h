/*
 * The library produced by the company was very much not up to par with any standards, so
 * this is my adaptation of the original Meccano library found on their website.
 * Feel free to use it for whatever you want. (;
 *
 * @author Alex Frederiksen
 */

#pragma once

/* for "byte" type and others */
#include "Arduino.h"

#define MAX_CHAIN 4

#define BIT_DELAY 417

#define TYPE_NONE  0x00
#define TYPE_SERVO 0x01
#define TYPE_LED   0x02

#define HEADER_BYTE  0xFF
#define NOMOD_BYTE   0xFE
#define NEWMOD_BYTE  0xFE
#define ERASE_BYTE   0xFD
#define REQUEST_BYTE 0xFC
#define NIL_BYTE     0xFB
#define LIM_BYTE     0xFA

#define SERVO_MIN 0x18
#define SERVO_MAX 0xE8

class ServoAdapter;
class LedAdapter;

/* convenience typedefs for users */
typedef ServoAdapter MeccanoServo;
typedef LedAdapter   MeccanoLed;

struct Module {
	int connected = 0;
	int justConnected = 0;

	byte lastInput = 0x00;
	byte type = TYPE_NONE;

	byte output1 = NIL_BYTE;
	byte output2 = NIL_BYTE;

	void onInput(byte input);
	byte getOutput();
	int isConnected();
	void connect(byte type);
	void disconnect();
};

class ModuleAdapter {

	protected:
		Module & module;

	public:
		ModuleAdapter(Module & module);

		int isConnected();
		int justConnected();
		virtual int checkType() = 0;
};

class ServoAdapter : public ModuleAdapter {

	public:
		ServoAdapter(Module & module);

		int getPosition();
		void setPosition(int pos);
		void setLim(int enable);
		void setColor(byte r, byte g, byte b);

		int checkType();
};

class LedAdapter : public ModuleAdapter {

	public:
		LedAdapter(Module & module);

		void setColor(byte r, byte g, byte b, byte fadetime);

		int checkType();

};

class Chain {
	int pwmPin;
	int moduleNum = 0;
	byte outputs[MAX_CHAIN];
	Module modules[MAX_CHAIN];

	byte calculateCheckSum();
	void sendByte(byte data);
	byte receiveByte();

	public:
		Chain(int pwmPin);
		void update();

		MeccanoServo getServo(int id);
		MeccanoLed getLed(int id);
};
