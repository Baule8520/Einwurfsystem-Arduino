/*
  Skript zum Einstellen der richtigen Zeit am DS3231

  By Paul Zech

  Basiert auf dem RTCLibrary Example von Adafruit
*/


#include "RTClib.h"                                         //Library: https://github.com/adafruit/RTClib

RTC_DS3231 RTC;                                             // Richtiges RTC Board auswählen und benennen

void setup () {

  Serial.begin(9600);                                       // Seriellen Monitor Starten --> Debugging
  while (!Serial) { ; }

  if (! RTC.begin()) {                                      // Fehlermeldung RTC Modul nicht angeschlossen
    Serial.println("RTC Modul nicht angeschlossen");
    while (1); }

  Serial.println("Zeit einstellen \n");                        
  RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));           // Zeit wird auf aktuelle Kompilierungszeit eingestellt
}

 
void loop () {                                              // Im Loop wird aktuelle Uhrzeit und Temparatur angezeigt
    
    DateTime now = RTC.now();

    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

    Serial.print("Temperatur: ");
    Serial.print(RTC.getTemperature());                     // RTC Modul hat Temperatur Sensor um Zeit möglichst genau zu halten bei Temparatur Unterschieden, die sich ja auf die Schwingung des Quarz auswirken.
    Serial.println(" C");
    Serial.println();
    
    delay(3000);
}
