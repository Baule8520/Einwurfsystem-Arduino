/*
  Programm Einwurfsystem

  By Paul Zech

  Version 2.6:
  - Überprüfen ob Methodenaufruf notwendig
  - Kleinere Fehler und Korrekturen
 */


#include <SPI.h>                                            // SPI Controller
#include <Ethernet.h>                                       // Ethernet Funktionalität
#include <Servo.h>                                          // Servo Funktion
#include "RTClib.h"                                         // RTC Library: https://github.com/adafruit/RTClib --> Kann über Strg + Umschalt + I im Manager hinzugefügt werden


byte mac[] = {0xA8, 0x61, 0x0A, 0xAE, 0x42, 0xD7};          // MAC Adresse des Ethernet Shield hier eintragen
IPAddress ip(192, 168, 188, 10);                            // Freie IP-Adresse hier eintragen
EthernetServer server(80);                                  // Ethernet Port hier eintragen, für HTTP ist 80 der Standardport
String readString;                                          // Erstellen eines String in dem die Kommunikation mit dem Client gespeichert wird

Servo servo;                                                // Name des Servo Motors
RTC_DS3231 RTC;                                             // RTC Board auswählen und benennen

byte SensorKlappe = 2;                                      // Anschluss des Sensors für die Klappe, also offen oder geschlossen (Sicherheitsfunktion)
byte SensorServoZu = 3;                                     // Anschluss des Sensors für die Endposition (geschlossen) des Motors (Statusüberwachung)
byte SensorServoOffen = 5;                                  // Anschluss des Sensors für die Endposition (geöffnet) des Motors (Statusüberwachung)
byte SensorEinwurf = 6;                                     // Anschluss des Sensors für die Zahl der eingeworfenen Gegenstände
byte SensorReset = 7;                                       // Anschluss des Sensors für den manuellen Reset der eingeworfenen Gegenstände
//byte freierInOut = 8;                                     // Anschluss als Reserve, noch nicht belegt und der letzte freie Anschluss am Board

byte LampeFehler = 14;                                      // Anschluss Kontrollampe für den Fehlerfall (Ein == Fehler)
byte LampeServo = 15;                                       // Anschluss Kontrolllampe für Servoanschlag (Ein == Klappe ist zu)
byte LampeAuto = 16;                                        // Anschluss Kontrollampe für Automatische Steuerung (Ein == Klappe wäre bei automatischer Steuerung offen)
byte LampeAutoIO = 17;                                      // Anschluss Kontrollampe ob Automatik aktiviert oder deaktiviert ist (Ein == Automatik aktiv)

bool Automatik = 1;                                         // Automatischen Modus aktivieren / deaktivieren
bool ZustandAutomatik = 1;                                  // Zustand der automatischen Steuerung (Klappe offen / geschlossen)
bool Fehler = 0;                                            // Gibt an ob ein Fehler aufgetreten ist
int Eingeworfen = 0;                                        // Anzahl der eingeworfenen Gegenstände
int EingeworfenInsgesamt = 0;                               // Anzahl der eigeworfenen Gegenstände seit letztem Boot
bool Prello = 0;                                            // Zwischenspeicher zum Entprellen der Taste der eingeworfenen Gegenstände
byte Auf = 127;                                             // Servo Position Offen (Zwischen 0 und 180 Grad)
byte Zu = 15;                                               // Servo Position Geschlossen (Zwischen 0 und 180 Grad)
byte Tag;                                                   // Variablen speichern die Zeit des letzten Reboots ab
byte Monat;
int Jahr;
byte Stunde;
byte Minute;


void setup() {

  servo.attach(9, 2, 1520);                                                         // Anschlusspin des Servos, Minimale und Maximale Frequenz in Microsekunden
  pinMode(SensorKlappe, INPUT_PULLUP);                                              // Sensoren als Input mit Pull Up Widerstand klassifizieren
  pinMode(SensorEinwurf, INPUT_PULLUP);
  pinMode(SensorServoZu, INPUT_PULLUP);
  pinMode(SensorServoOffen, INPUT_PULLUP);
  pinMode(SensorReset, INPUT_PULLUP);
  
  pinMode(LampeServo, OUTPUT);                                                      // LEDs als Output klassifizieren
  pinMode(LampeFehler, OUTPUT);
  pinMode(LampeAuto, OUTPUT);
  pinMode(LampeAutoIO, OUTPUT);
  
  Serial.begin(9600);                                                               // Seriellen Monitor Starten --> Debugging
  while (!Serial) { ; }
  Serial.println(F("Einwurfsystem 2.6\n"));                                         // Titel des Programms am seriellen Monitor ausgeben
  
  if (! RTC.begin()) {                                                              // Fehlermeldung: Kein RTC Modul
    Serial.println(F("RTC Modul nicht vorhanden"));
    while (1);}
  if (RTC.lostPower()) {
    Serial.println(F("RTC Modul hat die Zeit verloren, wird neu eingestellt!"));    // Hinweis: Zeit wurde eingestellt nachdem Modul ohne Strom
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));}
    
  DateTime now = RTC.now();
  Tag = now.day();                                                                  // Speichert die Zeit des letzten Reboots ab                                
  Monat = now.month();
  Jahr = now.year();
  Stunde = now.hour();
  Minute = now.minute();

  Ethernet.begin(mac, ip);                                                          // Start des Ethernet Zugriffs

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {                            // Fehlermeldung: Kein Ethernet Shield
    Serial.println(F("Ethernet Shield nicht aufgesteckt"));
    while (true) {
      delay(1); }}
  if (Ethernet.linkStatus() == LinkOFF) {                                           // Fehlermeldung: Keine Netzwerk Verbindung
    Serial.println(F("Kein Netzwerkkabel eingesteckt"));}
  
  server.begin();                                                                   // Start des Servers
  Serial.print(F("Server befindet sich auf "));
  Serial.println(Ethernet.localIP());                                               // IP Adresse des Servers ausgeben
  Serial.print(F("\n"));
}


void loop() {                                                                             

  EthernetClient client = server.available();                                           // Nach neuem Client suchen

  int Einwurf = digitalRead(SensorEinwurf);                                             // Sensor Abfrage: Einwurf (Pull Up: Standartsensorwert == 1)
  int Klappe = digitalRead(SensorServoZu);                                              // Sensor Abfrage: Zustand Endanschlag (Pull Up: Standartsensorwert == 1)
  int ResetE = digitalRead(SensorReset);

  if (Klappe == 1) {                                                                    // Gibt den Zustand der Klappe (offen / geschlossen) an der LED aus
              digitalWrite(LampeServo, HIGH);}
  if (Klappe == 0) {
              digitalWrite(LampeServo, LOW);}
   
  if (Einwurf == 0 && Prello == 0) {                                                    // Zählt bei Einwurf == 0 den Eingeworfen nach oben
              Eingeworfen += 1;
              EingeworfenInsgesamt += 1;
              Prello = 1;}
  if (Einwurf == 1 && Prello == 1) {                                                    // Entprellt die Taste um eine halbe Sekunde --> Hier anpassen, je nach Sensortyp
              delay(100);
              Prello = 0;}
  if (ResetE == 0) {Eingeworfen = 0;}                                                   // Zurücksetzten des Zählers durch Sensor (Schalter)

  if (Fehler == 1) {digitalWrite(LampeFehler, HIGH);}                                   // Fehler Lampe einschalten, wenn ein Fehler passiert ist
  
  if (Automatik == 1) {                                                                 // Wenn die Automatik läuft, dann...
    
    digitalWrite(LampeAutoIO, HIGH);                                                    // Kontrollampe Automatikmodus einschalten
    DateTime now = RTC.now();
      
      if (now.dayOfTheWeek() == 0 && digitalRead(SensorServoOffen) == 0)  {             // Sonntag (Amerikanisches System: 0 == Sonntag, 1 == Montag, ...)
          ServoWriteAuf();                                                              // An Tagen an denen die Klappe immer offen sein braucht man nur den Tag, keine Uhrzeit
          digitalWrite(LampeAuto, HIGH);
          ZustandAutomatik = 1;}
  
      if (now.dayOfTheWeek() == 1) {                                                    // Montag (Unterhalb kann man die Öffnungszeiten eintragen, in diesem Fall von 10:00 bis 12:59:59 und von 16:00 bis 18:59:59)
          if (now.hour() == 10 || now.hour() == 11 || now.hour() == 12 || now.hour() == 15 || now.hour() == 16 || now.hour() == 17 || now.hour() == 18) {
            if (digitalRead(SensorServoZu) == 0) {
              ServoWriteZu();                                                             
              digitalWrite(LampeAuto, LOW);
              ZustandAutomatik = 0;}}
          else if (digitalRead(SensorServoOffen) == 0) {
            ServoWriteAuf();
            digitalWrite(LampeAuto, HIGH);
            ZustandAutomatik = 1;}}
          
      if (now.dayOfTheWeek() == 2 && digitalRead(SensorServoOffen) == 0)  {             // Dienstag
           ServoWriteAuf();
           digitalWrite(LampeAuto, HIGH);
           ZustandAutomatik = 1;}
      
      if (now.dayOfTheWeek() == 3)  {                                                   // Mittwoch
        if (now.hour() == 10 || now.hour() == 11 || now.hour() == 12 || now.hour() == 13 || now.hour() == 14 || now.hour() == 15 || now.hour() == 16 || now.hour() == 17) {
            if (digitalRead(SensorServoZu) == 0) {
              ServoWriteZu();
              digitalWrite(LampeAuto, LOW);
              ZustandAutomatik = 0;}}
          else if (digitalRead(SensorServoOffen) == 0) {
            ServoWriteAuf();
            digitalWrite(LampeAuto, HIGH);
            ZustandAutomatik = 1;}}

       if (now.dayOfTheWeek() == 4)  {                                                  // Donnerstag
          if (now.hour() == 15 || now.hour() == 16 || now.hour() == 17) {
            if (digitalRead(SensorServoZu) == 0) {
              ServoWriteZu();
              digitalWrite(LampeAuto, LOW);
              ZustandAutomatik = 0;}}
          else if (digitalRead(SensorServoOffen) == 0) {
            ServoWriteAuf();
            digitalWrite(LampeAuto, HIGH);
            ZustandAutomatik = 1;}}
          
       if (now.dayOfTheWeek() == 5)  {                                                   // Freitag
          if (now.hour() == 10 || now.hour() == 11 || now.hour() == 12 || now.hour() == 15 || now.hour() == 16 || now.hour() == 17 || now.hour() == 18) {
            if (digitalRead(SensorServoZu) == 0) {
              ServoWriteZu();
              digitalWrite(LampeAuto, LOW);
              ZustandAutomatik = 0;}}
          else if (digitalRead(SensorServoOffen) == 0) {
            ServoWriteAuf();
            digitalWrite(LampeAuto, HIGH);
            ZustandAutomatik = 1;}}
          
      if (now.dayOfTheWeek() == 6 && digitalRead(SensorServoOffen) == 0)  {              // Samstag
            ServoWriteAuf();
            digitalWrite(LampeAuto, HIGH);
            ZustandAutomatik = 1;}}

  if (Automatik == 0) {digitalWrite(LampeAutoIO, LOW);}                                             
 
  if (client) {                                                                         // Wenn Client gefunden, dann...    
    
    Serial.println(F("Neuer Client verfügbar"));
    
    while (client.connected()) {
     
      if (client.available()) {

        DateTime now = RTC.now();                                                       // Liest aktuelle Uhrzeit im Moment der HTTP Abfrage aus
        char X = client.read();                                                         // Speichert die ankommenden Daten (jeweils nur ein Zeichen) im Zeichen (8bit) "X"
        if (readString.length() < 100) {                                                // Speichert den ganzen Request im String bis zu einer Länge von 100
          readString += X;}

        if (X == '\n') {                                                                // Wenn eine leere Zeile ankommt (Signalisiert das Ende eines HTTP Request)
                           
          client.println("HTTP/1.1 200 OK");                                            // Inhalt der "Website" --> HTML Header
          client.println("Content-Type: text/html");
          client.println();
          
          client.println("<!DOCTYPE HTML>");                                            // Inhalt der "Website" --> HTML Content
          client.println("<html>");
          client.println("<head>");
          client.println("<title>Steuerung</title>");
          client.println("</head>");
          client.println("<body>");
          client.println("<h1>Einwurfsystem</h1><br />");
          
          client.println(F("Letzte Aktualisierung um "));                               // Gibt die Zeit der letzten Aktualisierung der Website aus
          client.print(now.hour(), DEC);
          client.print(':');
          client.print(now.minute(), DEC);
          client.print(':');
          client.print(now.second(), DEC);
          client.print(" am ");
          client.print(now.day(), DEC);
          client.print('.');
          client.print(now.month(), DEC);
          client.print('.');
          client.print(now.year(), DEC);          
          client.println("<br /><br />");

          client.println(F("Bitte nach jeder Aktion unterhalb und um den aktuellen Status einzusehen die Seite aktualisieren: "));
          client.println("<a href=\"/?1aktual\"\">Aktualisieren</a><br /><br /><br />"); // Schaltfläche Aktualisieren --> ohne Aktion, lädt nur die Seite neu

          if (Fehler == 1) {                                                             // Ausgabe im Fehlerfall
            client.println(F("Es ist ein Fehler im Schliessmechanismus aufgetreten, bitte ueberpruefen und das System anschliessend am Geraet per Schalter resetten!<br /><br /><br />"));}
          
          client.print("<a href=\"/?1auto\"\">Automatische</a>");                        // href Verlinkung um Aktion auszuführen 
          client.print(" oder ");
          client.print("<a href=\"/?1manu\"\">Manuelle</a>");                            // href Verlinkung um Aktion auszuführen
          client.print(" Steuerung<br /><br />");
          
          if (Automatik == 0) {
            client.println("<a href=\"/?1auf\"\">Klappe Auf</a>");                       // href Verlinkung um Aktion auszuführen 
            client.print(" oder ");
            client.println("<a href=\"/?1zu\"\">Klappe Zu</a><br /><br />");}            // href Verlinkung um Aktion auszuführen

          client.println(F("Status Automatikmodus: Klappe ist "));                       // Zeigt an, ob die Automatik die Klappe öffnen oder sperren würde
            if (ZustandAutomatik == 1) {
              client.println("offen");}
            if (ZustandAutomatik == 0) {
              client.println("zu");}
          client.println("<br /><br />");

          client.println(F("Status aktuell: Klappe ist "));                              // Gibt den aktuellen Zustand der Klappe aus, basierend auf dem Sensor Wert
            if (Klappe == 0) {
              client.println("offen");}
            if (Klappe == 1) {
              client.println("zu");}
          client.println("<br /><br /><br />");
          
          client.println(F("Eingeworfene Gegenstaende: "));                              // Gibt die Anzahl von "Eingeworfen" aus
          client.println(Eingeworfen);
          client.println("  -->  ");
          client.println("<a href=\"/?1reset\"\">Reset des Zaehlers</a><br /><br />");   // Schaltfläche Reset des Zählers 

          client.println(F("Letzter Systemneustart war um "));                           // Gibt die Zeit des letzten Systemneustarts aus
          client.print(Stunde);
          client.print(':');
          client.print(Minute);
          client.print(" am ");
          client.print(Tag);
          client.print('.');
          client.print(Monat);
          client.print('.');
          client.print(Jahr);
          client.println(F(", seitdem eingeworfene Gegenstaende: "));                    // Gibt die Anzahl von "EingeworfenInsgesamt" aus
          client.println(EingeworfenInsgesamt);
          client.println("<br /><br /><br />");
          
          client.println(F("Systemtemperatur: "));                                       // Nett to know: Eine Temparaturanzeige des DS3231 Boards :-)
          client.print(RTC.getTemperature());
          client.println(" C");
          
          client.println("</body>");
          client.println("</html>");                                                     // Ende der HTML Informationen
          
          Serial.println(F("Daten abgesendet"));                                         // Information über erfolgreiche Sendung am seriellen Monitor

          delay(2);                                                                      // Warten bis Client Daten empfangen hat
    
          client.stop();                                                                 // Verbindung beenden

          if (readString.indexOf("?1auf") > 0){                                          // Servo auffahren, anhand von href im ankommenden HTTP String
              ServoWriteAuf(); }
               
          if (readString.indexOf("?1zu") > 0){                                           // Servo Zufahren, anhand von href im ankommenden HTTP String
              ServoWriteZu(); }  
                 
          if (readString.indexOf("?1reset") > 0){                                        // Reset des Zählers der eingeworfenen Gegenstände
                Eingeworfen = 0;
               Serial.println(F("Reset des Zählers"));} 

          if (readString.indexOf("?1auto") > 0){                                         // Automatik aktivieren
               Automatik = 1;
               Serial.println(F("Automatik aktiviert"));}
               
          if (readString.indexOf("?1manu") > 0){                                         // Automatik deaktivieren
                Automatik = 0;
               Serial.println(F("Automatik deaktiviert"));}       
               
          readString = "";                                                               // String entleeren
          }
      }
    }
    Serial.println(F("Verbindung geschlossen\n\n"));
  }
}


void ServoWriteAuf() {                                                                  // Methode zum Servo öffnen
  if (digitalRead(SensorKlappe) == 0) { 
     servo.write(Auf);
     Serial.println(F("Klappe Auf"));
     delay(1000);
      if (digitalRead(SensorServoOffen) == 0) {                                         // Wenn Klappe nicht innerhalb von 1 Sekunde offen ist --> Fehler
         Fehler = 1; }}}


void ServoWriteZu(){                                                                    // Methode zum Servo schließen
  if (digitalRead(SensorKlappe) == 0) { 
     servo.write(Zu);
     Serial.println(F("Klappe Zu"));
     delay(1000);
      if (digitalRead(SensorServoZu) == 0) {                                            // Wenn Klappe nicht innerhalb von 1 Sekunde zu ist --> Fehler
        Fehler = 1; }}}
