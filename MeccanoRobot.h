#ifndef MECCANO_ROBOT
#define MECCANO_ROBOT

#include <Arduino.h>
#include <stdint.h>

/* Header file for the Meccanoid library 
 *
 * @author Alexander Frederiksen
 * @version 06/05/17
 */

class ModuleType {
    public:
        static const uint8_t NO_DEVICE = 0x00;
        static const uint8_t SERVO =     0x01;
        static const uint8_t LED =       0x02;
};

class Chain;

class Module {
    protected:
        int id;
        uint8_t inputData;

    public:
        Module(int id);
        int getId() { return id; }
        virtual uint8_t getOutputData() = 0; // force subclasses to implement
        virtual uint8_t getType() = 0;
        virtual void setInputData(uint8_t data) { inputData = data; }
        uint8_t getInputData() { return inputData; }
};

typedef void (*NotifyHandler)(Module*);

class NoDevice : public Module {
    private:
        uint8_t outputData;

    public:
        NoDevice(int id);
        uint8_t getType() { return ModuleType::NO_DEVICE; }
        void requestModType();
        uint8_t getOutputData() { return outputData; }
};

class Servo : public Module {
    private:
        const uint8_t LIM_BYTE = 0xFA;
        const uint8_t POS_MIN = 0x18;
        const uint8_t POS_MAX = 0xE8;

        bool LIM;
        uint8_t position;
        uint8_t color; // last LSB bits correspond to RGB color
        bool changeColor; // flag to override normal operation

    public:
        Servo(int id);
        uint8_t getType() { return ModuleType::SERVO; }
        uint8_t getOutputData();
        void setInputData(uint8_t data);
        void setPosition(int pos);
        int getPosition();
        bool isLIM() { return LIM; }
        void setLIM();
        void setColor(uint8_t color);
        void setColor(bool red, bool green, bool blue);
        uint8_t getColor() { return color; }
};

class LED : public Module {
    private:
        uint8_t outputData1;
        uint8_t outputData2;
        int byteStep;
    
    public:
        LED(int id);
        uint8_t getType() { return ModuleType::LED; }
        void setColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t fadeTime);
        uint8_t getOutputData();
};

class Chain {   
    public:
        static const int MAX_MODS = 4;

    private:
        int pwmPin;
        int currentMod;
        Module* modules[MAX_MODS];
        NotifyHandler notifyHandlers[MAX_MODS]; // array of function pointers

        void sendData(uint8_t data);
        uint8_t receiveData();
        uint8_t calculateCheckSum(int dataSum);
    
    public:
        // declare static members
        static const uint8_t HEADER_BYTE = 0xFF;
        static const uint8_t DISCOVER_BYTE = 0xFE;
        static const uint8_t REQUEST_TYPE_BYTE = 0xFC;
        static const uint8_t DISCOVERED_COLOR = 0xF4;

        static const int UPDATE_DELAY = 10;    // in milleseconds
        static const int SEND_BIT_DELAY = 417; // in microseconds
        
        Chain(int pin);
        ~Chain();
        
        // getters and setters
        Module* getModule(int modId) { return modules[modId]; }
        int getCurrentModule() { return currentMod; }   
        void notifyWhenAvailable(int id, void (*handler)(Module*)); 
        
        // main update loop
        void update();
};

#endif
