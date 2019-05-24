#include <Meccanoid.h>

typedef MeccanoServo Servo;
typedef MeccanoLed   Led;

int pin = 8;

Chain chain(pin);

Servo servo1 = chain.getServo(0);
Servo servo2 = chain.getServo(1);
Led led =      chain.getLed(2);

int step = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  randomSeed(analogRead(0));
}

void loop() {
  // put your main code here, to run repeatedly:
  chain.update();

  updateServo(servo1, 1);
  updateServo(servo2, 2);

  if (led.justConnected()) {
    Serial.println("Led connected! Setting the color...");
    led.setColor(0x07, 0x01, 0x01, 0x00);
  }

  if (led.isConnected()) {
    if (step % 15 == 0) {
      led.setColor(random(8), random(8), random(8), 0x03);
    }
  }
  
  delay(200);
  
  step++;
}

void updateServo(Servo servo, int servoNum) {
  if (servo.justConnected()) {
    Serial.print("Servo ");
    Serial.print(servoNum);
    Serial.print(" : ");
    Serial.println("Servo connected! Enabling LIM mode...");
    servo.setPosition(120);
    chain.update();
    delay(500);
    servo.setLim(1);
    chain.update();
  }

  if (servo.isConnected()) {
    Serial.print("Servo ");
    Serial.print(servoNum);
    Serial.print(" : ");
    Serial.println(servo.getPosition());
    
    if (step % 10 == 0) {
      int c = (step / 10) % 8;
      servo.setColor(c, c >> 1, c >> 2);
      chain.update();
    }
  }
}
