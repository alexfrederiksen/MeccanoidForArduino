#include "MeccanoRobot.h"

/* A library to interface with the Meccano Meccanoid by an Arduino. It is 
 * an improved version of the original Meccano library that can be found on 
 * their website.
 *
 * @author Alexander Frederiksen
 * @version 06/05/17
 */

Module::Module(int id) {
	this -> id = id;
}

NoDevice::NoDevice(int id) : Module(id) {
	outputData = Chain::DISCOVER_BYTE;
}

void NoDevice::requestModType() {
	outputData = Chain::REQUEST_TYPE_BYTE;
}

Servo::Servo(int id) : Module(id) {
	setPosition(90);
}

void Servo::setLIM() {
	LIM = true;
}

void Servo::setPosition(int pos) {
	LIM = false;
	// map from degrees to bytes
	pos = constrain(pos, 0, 180);
	pos = map(pos, 0, 180, POS_MIN, POS_MAX);
	position = pos;
}

int Servo::getPosition() {
	return map(position, POS_MIN, POS_MAX, 0, 180);
}

void Servo::setColor(uint8_t color) {
	this -> color = color;
	changeColor = true;
}

void Servo::setColor(bool red, bool green, bool blue) {
	// set three LSB bits for given color 
	color = 0xF0;
	if (red) color |= 0x01;
	if (green) color |= 0x02;
	if (blue) color |= 0x04;
	changeColor = true;
}

void Servo::setInputData(uint8_t data) {
	// only use data if the servo is set in LIM mode
	if (LIM) position = data;
}

uint8_t Servo::getOutputData() {
	// check color
	if (changeColor) {
		changeColor = false;
		return color;
	}

	// do normal operation (return position unless LIM flag is set)
	return LIM ? LIM_BYTE : position;
}

LED::LED(int id) : Module(id) {
	byteStep = 0;
}

// all values are 3-bit
// fadeTime is between 0 - 4 seconds
void LED::setColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t fadeTime) {
	outputData1 = 0;
	outputData2 = 0;
	outputData1 = 0x3F & (((green << 3) & 0x38) | (red & 0x07));
	outputData2 = 0x40 | (((fadeTime << 3) & 0x38) | (blue & 0x07));
}

uint8_t LED::getOutputData() {
	byteStep = (byteStep + 1) % 2;
	if (byteStep == 0) {
		return outputData1;
	} else {
		return outputData2;
	}
}

Chain::Chain(int pin) {
	this -> pwmPin = pin;
	this -> currentMod = 0;
	
	// reset slots to "NoDevice" instances and reset notify pointers
	for (int i = 0; i < MAX_MODS; i++) {
		modules[i] = new NoDevice(i);
		notifyHandlers[i] = 0x00;
	}
}

void Chain::notifyWhenAvailable(int id, NotifyHandler handler) {
	notifyHandlers[id] = handler;
}

void Chain::update() {
	// send header byte
	sendData(HEADER_BYTE);
	// send module data and add to data sum
	int dataSum = 0;
	for (int i = 0; i < MAX_MODS; i++) {
		uint8_t data = modules[i] -> getOutputData();
		dataSum += data;
		sendData(data);
	}
	// calculate checksum then send
	sendData(calculateCheckSum(dataSum));
	// receive data from current module
	uint8_t input = receiveData();
	// define current module
	Module* current = modules[currentMod];	
	// do initializations if module is undefined
	if (current -> getType() == ModuleType::NO_DEVICE) {
		NoDevice* blank = static_cast<NoDevice*>(current);
		if (input == DISCOVER_BYTE) {
			// module exists, request the module type
			blank -> requestModType();
		}
		if (current -> getOutputData() == REQUEST_TYPE_BYTE) {
			// if previously requested type, set accordingly
			switch (input) {
				case ModuleType::SERVO:
					modules[currentMod] = new Servo(currentMod);
					break;
				case ModuleType::LED:
					modules[currentMod] = new LED(currentMod);
					break;
			}
			Module* newMod = modules[currentMod];
			if (newMod -> getType() != ModuleType::NO_DEVICE) {
				int id = newMod -> getId();
				if (notifyHandlers[id]) {
					notifyHandlers[id](newMod);
				}
			}
		}	
	} else {
		current -> setInputData(input);;
	}

	// increment the module counter
	currentMod++;
	if (currentMod >= MAX_MODS) currentMod = 0;
	// delay update loop
	// delay(UPDATE_DELAY);
}

void Chain::sendData(uint8_t data) {
	// set start conditions
	pinMode(pwmPin, OUTPUT);
	digitalWrite(pwmPin, LOW);
	delayMicroseconds(SEND_BIT_DELAY);
	// write bits individually
	for (uint8_t mask = 0x01; mask > 0; mask <<= 1) {
		digitalWrite(pwmPin, data & mask);
		delayMicroseconds(SEND_BIT_DELAY);
	}
	// set stop conditions
	digitalWrite(pwmPin, HIGH);
	delayMicroseconds(SEND_BIT_DELAY * 2);
}

uint8_t Chain::receiveData() {
	pinMode(pwmPin, INPUT);
	delay(1.5);
	
	uint8_t data = 0x00;
	for (uint8_t mask = 0x01; mask > 0; mask <<= 1) {
		if (pulseIn(pwmPin, HIGH, 2500) > 400) data |= mask;
	}

	return data;
}

uint8_t Chain::calculateCheckSum(int dataSum) {
	int checksum = dataSum;
	checksum += checksum >> 8;
	checksum += checksum << 4;
	checksum &= 0xF0;
	checksum |= currentMod & 0x0F;
	return checksum;
}
