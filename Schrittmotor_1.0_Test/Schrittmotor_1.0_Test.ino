/*
  
  Schrittmotor Test - Hardware: https://www.reichelt.de/arduino-motorsteuerung-inkl-schrittmotor-uln2003a-ard-step-motor-p255289.html?&trstct=pos_0&nbc=1

  By Paul Zech

  Basierend auf dem Stepper Beispiel "stepper_oneRevolution"

 */


#include <Stepper.h>                                // Schrittmotor Library einbinden --> https://www.arduino.cc/en/Reference/Stepper

Stepper Motor(1024, A5, A3, A4, A2);                // Namen des Motors festlegen (Schritte pro Umdrehung festlegen, Anschlusspins (1 bis 4) festlegen)


void setup() {
 
  Motor.setSpeed(1);}                               // Geschwindigkeit festlegen


void loop() {

  Motor.step(1024);                                 // Eine Umdrehung im Uhrzeigersinn
  delay(1000);

  Motor.step(-1024);                                // Eine Umdrehung gegen den Uhrzeigersinn
  delay(1000);}
