/*
  Fading

  This example shows how to fade an LED using the analogWrite() function.

  The circuit:
   LED attached from digital pin 9 to ground.

  Created 1 Nov 2008
  By David A. Mellis
  modified 30 Aug 2011
  By Tom Igoe

  http://www.arduino.cc/en/Tutorial/Fading

  This example code is in the public domain.

*/


int ledPin = 3;    // LED connected to digital pin 9

void setup() {
  // nothing happens in setup

  pinMode( 3, OUTPUT);
  pinMode( 5, OUTPUT);
  pinMode( 6, OUTPUT);
  pinMode( 9, OUTPUT);
  pinMode(10, OUTPUT);

  pinMode(13, OUTPUT);
}

void arm() {
  analogWrite( 3, 250); //aux
  analogWrite( 5, 250); //rud
  analogWrite(10, 10); //thr

  digitalWrite(13, HIGH);

  delay(5000);
}

void loop() {
  arm();

//
//  analogWrite( 3, 10); //aux
  analogWrite( 5, 128); //rud
  analogWrite( 6, 128); //ele
  analogWrite( 9, 128); //ail
  analogWrite(10, 250); //thr

  analogWrite(10, 250); //thr
  analogWrite( 6, 10); //ele

//
//  digitalWrite(13, LOW);
//
  delay(5000);

  analogWrite(10, 10); //thr

  delay(10000);
}


