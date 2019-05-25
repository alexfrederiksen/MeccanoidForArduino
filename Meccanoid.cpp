/*
 * The library produced by the company was very much not up to par with any standards, so
 * this is my adaptation of the original Meccano library found on their website.
 * Feel free to use it for whatever you want. (;
 *
 * @author Alex Frederiksen
 */

#include "Meccanoid.h"

/* -- Module -- */

void Module :: onInput(byte input) {
	lastInput = input;
}

byte Module :: getOutput() {
	byte output;
	// fallback on second data byte if first is already used 
	if (output1 == NIL_BYTE) {
		output = output2;
		output2 = NIL_BYTE;
	} else {
		output = output1;
		output1 = NIL_BYTE;
	}

	return output;
}

int Module :: isConnected() {
	return connected;
}

void Module :: connect(byte type) {
	justConnected = 1;
	connected = 1;
	this -> type = type;
}

void Module :: disconnect() {
	connected = 0;
	type = TYPE_NONE;
}

/* -- Module adapter -- */

ModuleAdapter :: ModuleAdapter(Module & module, Chain & chain) : module(module), chain(chain) {

}

void ModuleAdapter :: setAutoUpdate(bool enable) {
	autoUpdate = enable;
}

int ModuleAdapter :: isConnected() {
	return module.isConnected() && checkType();
}

int ModuleAdapter :: justConnected() {
	if (module.justConnected && checkType()) {
		module.justConnected = 0;
		return 1;
	} else {
		return 0;
	}
}

/* -- Servo adapter -- */

ServoAdapter :: ServoAdapter(Module & module, Chain & chain) : ModuleAdapter(module, chain) {

}

int ServoAdapter :: getPosition() {
	return map(module.lastInput, SERVO_MIN, SERVO_MAX, 0, 180);
}

ServoAdapter & ServoAdapter :: setPosition(int pos) {
	module.output1 = map(pos, 0, 180, SERVO_MIN, SERVO_MAX);

	if (autoUpdate) chain.update();

	return *this;
}

ServoAdapter & ServoAdapter :: setLim(int enable) {
	if (enable)
		module.output1 = LIM_BYTE;
	else
		setPosition(getPosition());

	if (autoUpdate) chain.update();

	return *this;
}

ServoAdapter & ServoAdapter :: setColor(byte r, byte g, byte b) {
	// clamp parameters to 0 or 1 (take LSB)
	r &= 0x01;
	g &= 0x01;
	b &= 0x01;

	module.output1 = 0xF0 | (r << 0) | 
	                        (g << 1) | 
	                        (b << 2);

	if (autoUpdate) chain.update();

	return *this;
}

int ServoAdapter :: checkType() {
	return module.type == TYPE_SERVO;
}

/* -- Led adapter -- */

LedAdapter :: LedAdapter(Module & module, Chain & chain) : ModuleAdapter(module, chain) {

}

/*
 * r, g, b can be values 0 through 7 (0 being the darkest)
 *
 * fadetime values:
 *
 * val  seconds
 *  0    0.0
 *  1    0.2
 *  2    0.5 
 *  3    0.8
 *  4    1.0
 *  5    2.0
 *  6    3.0
 *  7    4.0
 *
 */

LedAdapter & LedAdapter :: setColor(byte r, byte g, byte b, byte fadetime) {
	module.output1 = 0x3F & (((g << 3) & 0x38) | (r & 0x07));
	module.output2 = 0x40 | (((fadetime << 3) & 0x38) | (b & 0x07));

	if (autoUpdate) {
		chain.update();
		chain.update();
	}

	return *this;
}

int LedAdapter :: checkType() {
	return module.type == TYPE_LED;
}

/* -- Chain -- */

Chain :: Chain(int pwmPin) : pwmPin(pwmPin) {
	pinMode(pwmPin, OUTPUT);

	for (int i = 0; i < MAX_CHAIN; i++)
		outputs[i] = NOMOD_BYTE;
}

void Chain :: update() {
	// send header
	sendByte(HEADER_BYTE);
  
	// send output data
	for (int i = 0; i < MAX_CHAIN; i++) {
		if (modules[i].isConnected()) {
			byte output = modules[i].getOutput();
			if (output != NIL_BYTE)
				outputs[i] = output;
		}

		sendByte(outputs[i]);
	}

	// calculate and send checksum
	byte checkSum = calculateCheckSum();
	sendByte(checkSum);

	Module & module = modules[moduleNum];

	// receive input from current module
	byte input = receiveByte();

	// if received back 0xFE, then the module exists so get ID number
	if (input == NEWMOD_BYTE)
		outputs[moduleNum] = REQUEST_BYTE;
  
	if (!module.isConnected()) {
		// if received an input type, then connect module
		if (input == TYPE_SERVO || input == TYPE_LED)
			module.connect(input);
	} else {
		module.onInput(input);
	}

	// if received 0x00 then do full reset?
	if (input == 0x00) {
		module.disconnect();
		outputs[moduleNum] = NOMOD_BYTE;
	}
	
	// increment module index
	moduleNum = (moduleNum + 1) % MAX_CHAIN;

	// wait 10ms and keep things happy
	delay(10);
}

MeccanoServo Chain :: getServo(int id) {
	return ServoAdapter(modules[constrain(id, 0, MAX_CHAIN - 1)], *this);
}

MeccanoLed Chain :: getLed(int id) {
	return LedAdapter(modules[constrain(id, 0, MAX_CHAIN - 1)], *this);
}

void Chain :: sendByte(byte data) {
	pinMode(pwmPin, OUTPUT);

	// start bit - 417us LOW
	digitalWrite(pwmPin, LOW);
	delayMicroseconds(BIT_DELAY);


	for (byte mask = 0x01; mask > 0; mask <<= 1) {
		if (data & mask)
			digitalWrite(pwmPin, HIGH);
		else
			digitalWrite(pwmPin, LOW);

		delayMicroseconds(BIT_DELAY);
	}


	// stop bits - 417us HIGH
	digitalWrite(pwmPin, HIGH);
	delayMicroseconds(BIT_DELAY);

	digitalWrite(pwmPin, HIGH);
	delayMicroseconds(BIT_DELAY);

}

byte Chain :: receiveByte() {
	byte tempByte = 0;

	pinMode(pwmPin, INPUT);

	delay(1.5);

	for (byte mask = 0x01; mask > 0; mask <<= 1) {
		if (pulseIn(pwmPin, HIGH, 2500) > 400)
			tempByte = tempByte | mask;
	}

	return tempByte;
}

byte Chain :: calculateCheckSum() {
	int sum = 0;

	for (int i = 0; i < MAX_CHAIN; i++)
		sum += outputs[i];

	sum = sum + (sum >> 8);
	sum = sum + (sum << 4);
	sum = sum & 0xF0;
	sum = sum | moduleNum;

	return sum;
}

