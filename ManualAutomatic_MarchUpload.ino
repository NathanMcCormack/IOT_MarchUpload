/*
Name: Nathan McCormack
Date: 16/03/2024
Project: Automated Blinds
Code Description: This code resembles an automated blind system. It has two modes - Manual/Automatic which can be selected using a toggle switch.
The manual mode uses two push buttons to move the blind up/down. The automatic mode uses an LDR to detect when it is dark/bright outside and the motor
will move the blind up/down accordingly. The ultasonic sensor detects when the blind is at the bottom and stops the motor from moving it too far.
Upon startup the blind height is calculated using the amount of revolutions it takes the stepper motor for the blind to reach the bottom.
*/
#include <Stepper.h>

//Stepper Motor
const int stepsPerRevolution = 2048;
Stepper myStepper(stepsPerRevolution, 18, 12, 19, 13);

//Ultrasonic Sensor
const int TRIG_PIN = 5;
const int ECHO_PIN = 16;
long duration;
float cms;
int blindHeight = 1, x = 0, blindPosition = 0;

//LDR
#define LIGHT_SENSOR_PIN 4

//DHT11
#include <DFRobot_DHT11.h>
DFRobot_DHT11 DHT;
#define DHT11_PIN 25

//Push Buttons
const int Btn1 = 21;
const int Btn2 = 14;
int Bdelay1 = 0; //these delays are used to prevent false signals being sent to each pin for the buttons
int Bdelay2 = 0;

//Toggle Switch
const int SW1 = 34;
const int SW2 = 32;
int SWdelay1 = 0; //these delays are used to prevent false signals being sent to each pin for the switch
int SWdelay2 = 0;

void Ultrasonic();
void Clockwise();
void CounterClockwise();
void LDR();
void DHT11();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  myStepper.setSpeed(10);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LIGHT_SENSOR_PIN, INPUT);
  pinMode(SW1, INPUT);
  pinMode(SW2, INPUT);
  pinMode(Btn1, INPUT);
  pinMode(Btn2, INPUT);

  //Blinds go down on startup to calculate total length of window
  CounterClockwise();  //the ultrasonic wont get the distance unless this runs first
  Ultrasonic();   //gets initial distance from bind to windowsil


  while (cms >= 5) {
    CounterClockwise();  //blind goes down
    x++;
    blindHeight++;  //will get the total number of revolutions needed to reach th efull distance
    Ultrasonic();   //Height is checked again after 1 revolution of stepper motor
  }

  while (x >= 0) {
  Clockwise();
    x--;
  }

  Ultrasonic();
}

void loop() {
  // put your main code here, to run repeatedly:
  int SWstate1 =digitalRead(SW1);
  int SWstate2 = digitalRead(SW2);
  int Bstate1 = digitalRead(Btn1);
  int Bstate2 = digitalRead(Btn2);

  if(SWstate1 == HIGH){
    delay(1);
    SWdelay1++;
      if(SWdelay1 > 20){
          if (Bstate1 == HIGH) {
        delay(1);
        Bdelay1++;
        if(Bdelay1>35){ //if the button is pressed for 35ms the motor will run, this prevents false signals
          CounterClockwise();
          Bdelay1=0; 
          Bdelay2=0;
        }
      }

      else if(Bstate2 == HIGH ){
        delay(1);
        Bdelay2++;
        if(Bdelay2>35){
          Clockwise();
          Bdelay1=0;
          Bdelay2=0;
        }
      }
       SWdelay1=0;
       SWdelay2=0;
    }
  }

  else if(SWstate2 == HIGH){
    delay(1);
    SWdelay2++;
    if(SWdelay2 > 30){
       LDR();
       DHT11();
       SWdelay1=0;
       SWdelay2=0;
    }
  }
}

void Clockwise() {
  myStepper.step(stepsPerRevolution);
}

void CounterClockwise() {
  myStepper.step(-stepsPerRevolution);
}

void Ultrasonic() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(5);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH);
  cms = (duration / 2) / 29.1;
  Serial.print(cms);
  Serial.println();
  delay(500);
}

void LDR() {
int analogValue = analogRead(LIGHT_SENSOR_PIN);

  if (analogValue < 20) {
    Serial.println("Brightness: ");
    Serial.println(analogValue);
    Serial.println("Room is dark, Blinds closed");

    if (blindPosition == 0) {//Blind is up
          if(analogValue <20){//Room is dark
          for (int y = blindHeight; y > 0; y--) {  //while y is less than the blind height the motor will keep running
            CounterClockwise();
            blindPosition++;  //tells us the blindPosition to prevent the blind from going down again once the main loop restarts
          }
        }
      }
  delay(500);
}
  else if (analogValue > 20) {
    Serial.println("Brightness: ");
    Serial.println(analogValue);
    Serial.println("Room is bright, Blinds open");

    if (blindPosition > 0) {  //Blinds are down
    if(analogValue >20){//Room is bright
      for (int z = 0; z < blindHeight; z++) {  //Motor will keep running unitl its reached the blind height value
        Clockwise();
        //blindPosition--;//Will reset the blind position to 1, when its fully up.
      }
    blindPosition = 0;
    }
  }
  }
  delay(500);
}
  

void DHT11(){
  DHT.read(DHT11_PIN);
  Serial.print("temp:");
  Serial.print(DHT.temperature);
  Serial.print("  humi:");
  Serial.println(DHT.humidity);
  delay(1000);
}
