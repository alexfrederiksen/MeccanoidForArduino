# MeccanoidForArduino
A C++ library for interfacing with the Meccano Meccanoid. If you have any questions, leave them in the issues tab. If you feel you can make this better, pull requests are greatly respected.

## Include in your Arduino project
To include the library, download the ZIP file and open Arduino IDE. In the Arduino IDE navigate Sketch -> Include Library -> Add .ZIP Library and select the library ZIP file. If you just want to get started, look in the testing folder for some example code. Otherwise, the rest of this document will show you the various things you can do.

## Create a module chain
Smart Modules can either be servos or LEDs. They are commonly daisy-chained for simplicity and can be controlled with a single PWM pin on your microcontroller. There can be a maximum of 4 modules in a single chain. The module closest to the pin has an id of 0 while the furthest has an id of 3. To create a chain you only need the PWM pin that the chain connects to. Refer to the [Smart Module Protocol](http://cdn.meccano.com/open-source/Meccano_SmartModuleProtocols_2015.pdf) (SM protocol) for specifications of pull-up resistors.
```c++
Chain chain(int pwmPin);
```

## Declaring modules
To start using the modules in your chain, you must first get their controllers:
```c++
...

int chainPin = 8; // can be any PWM pin

Chain chain(chainPin);

MeccanoServo servo1 = chain.getServo(0); // servo with id 0
MeccanoServo servo2 = chain.getServo(1); // servo with id 1
MeccanoLed led =      chain.getLed(2);   // led with id 2

...
```
Here we're using the `Chain::getServo(int id)` and `Chain::getLed(int id)` functions for getting controllers for the modules.

## Using modules
A key thing to note about modules is that the Arduino gets input from each every 4 calls to `Chain::update()` due to the SM protocol (it has to do with the max modules that can be on a chain). This means it could take time for the Arduino to find the modules on the chain, thus when modules are first declared they are in a disconnected state. Check the `ModuleAdapter::isConnected()` and `ModuleAdapter::justConnected()` methods to know when your module is ready for use.

## Using the servo module
Servo modules can rotate from 0 to 180 degrees and will lock at these positions. They can be set into LIM mode (Learned Intelligent Movement) in which the servos will unlock and can be physically moved. In this mode, they will also provide input from their encoders. Servos also have LEDs that you can change the color of.

### Moving servos
```c++
...

void loop() {
  // update chain
  chain.update();
  
  if (servo1.justConnected()) {
    // set color to red (r: 1, g: 0, b: 0) and pos to 45 degrees
    servo1.setColor(1, 0, 0)
          .setPosition(45);
  }
}

...
```
Note we used `ModuleAdapter::justConnected()` here to see if the previous update found the module. This way the if case only runs once at the servo's discovery. We also use `MeccanoServo::setColor(byte r, byte g, byte b)` where each parameter is 0 or 1, enabling the respective color. Mind the position argument of `MeccanoServo::setPosition(int pos)` can be between 0 and 180 where 0 is fully-clockwise.

### LIM mode
```c++
...

void loop() {
  // update chain
  chain.update();
  
  if (servo2.justConnected()) {
    // set color to red (r: 1, g: 0, b: 0) and lim mode
    servo2.setColor(1, 0, 0)
          .setLim(true);
  }
  
  if (servo2.isConnected()) {
    Serial.println(servo2.getPosition());
  }
}

...
```
A thing to note here is that servo's must be in LIM mode to give you sensible values. Also as soon as `MeccanoServo::setPosition` is invoked, LIM mode is disabled and the servo locks as usual.

## Using the LED module
The LED module can be set to colors with different fade times for transitioning. 

### Setting colors
This is the only special method of this class. Each color value (RGB) ranges from 0 to 7 and represents the brightness of that color (7 being the brightest). The fade time also ranges from 0 to 7 and represents no transition to a 4 second transition from the previous color respectively.
```c++
...

void loop() {
  // update chain
  
  if (led.justConnected()) {
    // set led to greenish color (r: 0 / 7, g: 7 / 7, b: 1 / 7) with (4 / 7) fade time
    led.setColor(0, 7, 1, 4);
  }
}

...
```

## Advance module usage
For any change to occur (e.g. color changes), a call to `Chain::update` must occur immediately after setting that property. But as we've seen so far, this isn't necessary. Well under the hood, `ModuleAdapter`'s default to doing just that. This was more a measure of convenience than anything and if need be, can be disabled with `ModuleAdapter::setAutoUpdate(bool enable)`. 
