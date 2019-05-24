#include <Meccanoid.h>

/* typedefs for convenience */
typedef MeccanoServo Servo;
typedef MeccanoLed   Led;

/* Note that their needs to be a 22k ohm pull-resitor on chain input,
 * See http://www.meccano.com/meccanoid-opensource Smart Module Protocol 
 * (bottom of PDF) for more details. */
 
/* PWM pin for chain */
int pin = 8;

/* define chain and PWM pin for it */
Chain chain(pin);

/* define modules to use and their positions */
Servo servo1 = chain.getServo(0); // first on chain  (0)
Servo servo2 = chain.getServo(1); // second on chain (1)
Led led =      chain.getLed(2);   // third on chain  (2)

int step = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  randomSeed(analogRead(0));
}

void loop() {
  // update chain to poll for input
  chain.update();

  // invoke our local servo updator method
  updateServo(servo1, 1);
  updateServo(servo2, 2);

  if (led.justConnected()) {
    Serial.println("Led connected! Setting the color...");
    led.setColor(0x07, 0x01, 0x01, 0x00);
  }

  if (led.isConnected()) {
    // change to random color every 15 steps
    if (step % 15 == 0) {
      led.setColor(random(8), random(8), random(8), 0x03);
    }
  }
  
  delay(200);
  
  step++;
}

void updateServo(Servo servo, int servoNum) {
  if (servo.justConnected()) {

    // log status
    Serial.print("Servo ");
    Serial.print(servoNum);
    Serial.print(" : ");
    Serial.println("Servo connected! Enabling LIM mode...");

    // set color and move servo
    servo.setColor(1, 0, 0)
         .setPosition(120);
         
    // let servo reach position
    delay(500);
    
    // make servo floppy
    servo.setLim(1);
  }

  if (servo.isConnected()) {

    // log status
    Serial.print("Servo ");
    Serial.print(servoNum);
    Serial.print(" : ");
    Serial.println(servo.getPosition());

    // change colors every 10 steps
    if (step % 10 == 0) {
      int c = (step / 10) % 8;
      servo.setColor(c, c >> 1, c >> 2);
    }
  }
}
