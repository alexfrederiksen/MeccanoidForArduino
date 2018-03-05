#include <MeccanoRobot.h>
#include <stdio.h>

/**
 * Perform a number of trials to read data from servos in LIM mode.
 * Store data in individual buffer then print out data after trial completions
 * 
 * @author Alexander Frederiksen
 * @version 3/4/2018
 */


#define CHAIN_PIN         6
#define SERVO_BUFFER_SIZE 100
#define TRIALS            50

Chain chain(CHAIN_PIN);
Servo * servo1 = 0;
Servo * servo2 = 0;

int trialCount = 0;

int servo1_buffer[SERVO_BUFFER_SIZE];
int servo2_buffer[SERVO_BUFFER_SIZE];

void pushBuffer(int * buf, int buf_size, int value) {
  // shift everything to the right
  for (int i = 1; i < buf_size; i++) {
    buf[i] = buf[i - 1];
  }
  // push in the value
  buf[0] = value;
}

void printBuffer(int * buf, int buf_size) {
  for (int i = 0; i < buf_size; i++) {
    Serial.println(buf[i]); 
  }
}

void handleModule1(Module * module) {
  if (module -> getType() == ModuleType::SERVO) {
    Serial.println("Found servo 1.");
    servo1 = (Servo *) module;
    servo1 -> setColor(false, true, false);
    servo1 -> setLIM();
  } else {
    Serial.println("Module 1 was not a servo.");
  }
}

void handleModule2(Module * module) {
  if (module -> getType() == ModuleType::SERVO) {
    Serial.println("Found servo 2.");
    servo2 = (Servo *) module;
    servo2 -> setColor(false, true, false);
    servo2 -> setLIM();
  } else {
    Serial.println("Module 2 was not a servo.");
  }
}

void handleModule3(Module * module) {

}

void handleModule4(Module * module) {
  
}

void setup() {
  // setup all the module init callbacks 
  chain.notifyWhenAvailable(0, handleModule1);
  chain.notifyWhenAvailable(1, handleModule2);
  chain.notifyWhenAvailable(2, handleModule3);
  chain.notifyWhenAvailable(3, handleModule4);

  // clear buffers
  memset((char *) servo1_buffer, 0, SERVO_BUFFER_SIZE * sizeof(servo1_buffer[0]));
  memset((char *) servo2_buffer, 0, SERVO_BUFFER_SIZE * sizeof(servo2_buffer[0]));
}

void loop() {
  // perform chain update
  chain.update();
  
  if (servo1) {
    int pos = servo1 -> getPosition();
    pushBuffer(servo1_buffer, SERVO_BUFFER_SIZE, pos);
    Serial.print("Servo 1 position:");
    Serial.println(pos);
  }
  if (servo2) {
    int pos = servo2 -> getPosition();
    pushBuffer(servo2_buffer, SERVO_BUFFER_SIZE, pos);
    Serial.print("Servo 2 position:");
    Serial.println(pos);
  }
  // for debugging
  delay(500);
  trialCount++;

  if (trialCount >= TRIALS) {
    // print out servo histories
    Serial.println("Servo 1 history:");
    printBuffer(servo1_buffer, SERVO_BUFFER_SIZE);
    Serial.println("\nServo 2 history:");
    printBuffer(servo2_buffer, SERVO_BUFFER_SIZE);
    
    // wait for Arduino reset
    while (true) delay(500);
  }
}
