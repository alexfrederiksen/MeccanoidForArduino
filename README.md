# MeccanoidForArduino
A library written in C++ to allow an Arduino to interface with the Meccanoid Smart Modules. It is an improved form of the one given by Meccano and includes many more capabilities. You can find the original Meccano library [here](http://www.meccano.com/meccanoid-opensource).

## Include in your Arduino project
To include the library, download the source files to your arduino library folder (the one in your workspace). The library source files should be in their own folder named "MeccanoidForArduino". This way it can be added as a library in the Arduino IDE through Sketch -> Include Library -> Add .ZIP Library and then select the folder containing the source files.

## Create a module chain
Smart Modules can either be servos or LEDs. They are commonly daisy-chained for simplicity and can be controlled with a single pin on your microcontroller. There can be a maximum of To create a chain you must give the pin.
```c++
Chain* chain = new Chain(int pwmPin);
```
### Methods of the Chain class
Method | Description | Example
-------|-------------|--------
`Chain(int pin)` | initializes a Chain instance | `Chain* chain = new Chain(6);`
`Module* getModule(int id)` | returns module with given id (0 - 3) | `Module* mod = chain -> getModule(0);`
`int getCurrentModule()` | return the id of the module that will get data on next update | `int id = chain -> getCurrentModule();`
`void notifyWhenAvailable(int id, void (*handler)(Module*))` | attached the handler to the id and calls it when the module is found | `chain -> notifyWhenAvailable(0, handlerForMod1);`
`void update()` | updates data between phyical modules and the class | `chain -> update();`

## Find modules
All modules are initialized as `NoDevice` instances. The chain's `update()` method should be called to search for modules. They will be found one at time so make sure to call it multiple times. To know a module has been found you can either check if a module exists after each update or you can attach a notification handler.
```c++
void loop() {
  chain -> update();
  if (chain.getModule(id) -> getType() != ModuleType::NO_DEVICE) {
    // do something with module
  }
}
```
#### Or preferably
```c++
void setup() {
  chain = new Chain(pwmPin);
  chain -> notifyWhenAvailable(id, handler);
}

void handler(Module* mod) {
  // do something with module
}
```

### Methods of the Module class
Method | Description | Example
-------|-------------|--------
`int getId()` | return the id of the module (0 - 3) | `int id = mod -> getId();`
`uint8_t getType()` | return the type of the module (`ModuleType::[NO_DEVICE, SERVO, LED]`) | `bool isServo = mod -> getType() == ModuleType::SERVO;`

## Use servo module
Servo modules can rotate from 0 to 180 degrees and will lock at these positions. They can be set into LIM mode (Learned Intelligent Movement) in which the servos will unlock and can be physically moved. In this mode, they will also provide input from their encoders. By default, the servos are set to the 90 degree position and the color is green. 

### Set position
The `position` is clamped to 0 to 180 degrees. This will disable LIM once processed in `update()`.
```c++
servo -> setPosition(int position);
```
### Get position
This will return data from the encoder if LIM is enabled or the last `setPosition` call otherwise.
```c++
int pos = servo -> getPosition();
```
### Set LIM mode
This allows encoder data to be colllected from the servo modules
```c++
servo -> setLIM();
```
### Set color
This change will be sent to the servo as top priority on the next update.
```c++
servo -> setColor(bool red, bool green, bool blue);
```
#### Or
```c++
servo -> setColor(uint8_t color); /// LSB bits represent red, blue, and green respectively
```
### Example
```c++
Servo* servo = 0; // hold servo pointer globally
void handler(Module* mod) {
  if (mod -> getType() == ModuleType::SERVO) {
    servo = static_cast<Servo*>(mod); // cast the Module* to a Servo*
    servo -> setPosition(45); // set the servo to the 45 degree position
    servo -> setColor(true, false, false); // set the color to red (the boolean values represent RGB)
    servo -> setLIM(); // will override previous position set
  }
}

void loop() {
  chain -> update();
  if (servo) {
    Serial.println(servo -> getPosition()); // will print the most recent data from the servo encoders
  }
  delay(500);
}
```
### Methods of the Servo class
Method | Description | Example
-------|-------------|--------
`void setPosition(int pos)` | set the position from 0 to 180 degrees | `servo -> setPosition(90);`
`int getPosition()` | return the position of servo | `int pos = servo -> getPosition();`
`void setLIM()` | enabled LIM mode on the servo | `servo -> setLIM();`
`bool isLIM()` | return if LIM is enabled | `bool LIM = servo -> isLIM();`
`void setColor(bool red, bool green, bool blue)` | sets the module color | `servo -> setColor(true, false, false);`
`void setColor(uint8_t color)` | set the module color | `servo -> setColor(0x02);`
`uint8_t getColor()` | return the last `setColor` call | `uint8_t color = servo -> getColor();`

## Use LED module
The LED module can be set to colors and fade times for transitioning. It takes two updates for the the changes to take into effect because each value (RGB and fade time) is 3 bits, taking a total of 12 bits to send information. Only a byte can be sent at a time for each module. 

### Set color
This is really the only special method of this class. It takes a total of 4 bytes, but only LSB 3 bits are used of each byte. Each color value (RGB) ranges from 0x00 to 0x07 and represents the brightness of that color (0x07 being the brightest). The fade time also ranges from 0x00 to 0x07 and represents no transition to a 4 second transition from the previous color respectively.
```c++
led -> setColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t fadeTime);
```
### Example
```c++
LED* led = 0;
void handler(Module* mod) {
  if (mod -> getType() == ModuleType::LED) {
    led = static_cast<LED*>(mod);
    led -> setColor(0x07, 0x00, 0x00, 0x05) // set the color to red with about a ~2 second transition
  }
}
```


