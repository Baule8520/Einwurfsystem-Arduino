/*
  Programm Einwurfsystem

  By Paul Zech

  Version 2.2:
  Enhancement: 
    - Eingeworfene Gegenstände seit letztem Neustart mit Anzeige wann der letzte Neustart denn war
    - Reset eingeworfener Gegenstände durch Schalter möglich
    - Status LED: Automatik Ein oder Aus
    - Mehr Statusinformationen im Browser
    - Optimierung der Variablentypen (bool, byte, ...)
    - Speicherplatzoptimierung im HTML Teil
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

byte SensorKlappe = 2;                                      // Anschluss des Sensors für die Klappe (Sicherheitsfunktion)
byte SensorServo = 3;                                       // Anschluss des Sensors für die Endposition (geschlossen) des Motors (Statusüberwachung)
byte SensorEinwurf = 5;                                     // Anschluss des Sensors für die eingeworfenen Gegenstände
byte SensorReset = 6;                                       // Anschluss des Schalters für den manuellen Reset der eingeworfenen Gegenstände
byte LampeServo = 7;                                        // Anschluss Kontrolllampe für Servoanschlag (Ein == Klappe ist zu)
byte LampeAuto = 8;                                         // Anschluss Kontrollampe für Automatische Steuerung (Ein == Klappe wäre bei automatischer Steuerung offen)
byte LampeAutoIO = 9;                                       // Anschluss Kontrollampe ob Automatik aktiviert oder deaktiviert ist (Ein == Automatik aktiv)

bool Automatik = 1;                                         // Automatischen Modus aktivieren
bool ZustandAutomatik = 1;                                  // Zustand der automatischen Steuerung
int Eingeworfen = 0;                                        // Anzahl der eingeworfenen Gegenstände
int EingeworfenInsgesamt = 0;                               // Anzahl der eigeworfenen Gegenstände seit letztem Boot
bool Einwurf = 1;                                           // Zwischenspeicher 1 zum Entprellen der Taste der eingeworfenen Gegenstände
bool Prello = 0;                                            // Zwischenspeicher 2 zum Entprellen der Taste der eingeworfenen Gegenstände
byte Auf = 20;                                              // Servo Position Offen (Zwischen 0 und 180 Grad)
byte Zu = 160;                                              // Servo Position Geschlossen (Zwischen 0 und 180 Grad)
byte Tag;                                                   // Variablen speichern die Zeit des letzten Reboots ab
byte Monat;
int Jahr;
byte Stunde;
byte Minute;


void setup() {

  servo.attach(A0);                                                                 // Anschluss des Servos
  pinMode(SensorKlappe, INPUT_PULLUP);                                              // Sensoren als Input mit Pull Up Widerstand klassifizieren
  pinMode(SensorEinwurf, INPUT_PULLUP);
  pinMode(SensorServo, INPUT_PULLUP);
  pinMode(SensorReset, INPUT_PULLUP);
  pinMode(LampeServo, OUTPUT);                                                      // LEDs als Output klassifizieren
  pinMode(LampeAuto, OUTPUT);
  pinMode(LampeAutoIO, OUTPUT);
  
  Serial.begin(9600);                                                               // Seriellen Monitor Starten --> Debugging
  while (!Serial) { ; }
  Serial.println(F("Einwurfsystem 2.2\n"));                                         // Titel des Programms ausgeben
  
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

  int Safe = digitalRead(SensorKlappe);                                                 // Sicherheitsfunktion, Servo wird nur gefahren wenn Klappe zu, also Safe == 1
  int Einwurf = digitalRead(SensorEinwurf);                                             // Liest den Sensor kontinuierlich ein (Pull Up: Standartsensorwert == 1)
   
    if (Einwurf == 0 && Prello == 0) {                                                  // Zählt bei Einwurf == 0 den Eingeworfen nach oben
              Eingeworfen += 1;
              EingeworfenInsgesamt += 1;
              Einwurf = 1;
              Prello = 1;}
    if (Einwurf == 1 && Prello == 1) {                                                  // Entprellt die Taste um eine halbe Sekunde --> Hier anpassen, je nach Sensortyp
              delay(500);
              Prello = 0;}

  if (digitalRead(SensorReset) == 0) {Eingeworfen = 0;}                                 // Zurücksetzten des Zählers durch Sensor (Schalter)

  if (Automatik == 1) {                                                                 // Wenn die Automatik läuft, dann...

    digitalWrite(LampeAutoIO, HIGH);                                                    // Kontrollampe Automatikmodus einschalten
    DateTime now = RTC.now();
      
      if (now.dayOfTheWeek() == 0)  {                                                   // Sonntag (Amerikanisches System: 0 == Sonntag, 1 == Montag, ...)
       if (Safe == 1) { 
        servo.write(Auf);                                                               // An Tagen an denen die Klappe immer offen sein braucht man nur den Tag, keine Uhrzeit
        digitalWrite(LampeAuto, HIGH);
        ZustandAutomatik = 1;}}
      
      if (now.dayOfTheWeek() == 1) {                                                    // Montag (Unterhalb kann man die Öffnungszeiten eintragen, in diesem Fall von 10:00 bis 12:59:59 und von 16:00 bis 18:59:59)
        if (Safe == 1) {
          if (now.hour() == 10 || now.hour() == 11 || now.hour() == 12 || now.hour() == 15 || now.hour() == 16 || now.hour() == 17 || now.hour() == 18) {
            servo.write(Zu);                                                             
            digitalWrite(LampeAuto, LOW);
            ZustandAutomatik = 0;}
          else {
            servo.write(Auf);
            digitalWrite(LampeAuto, HIGH);
            ZustandAutomatik = 1;}}}
          
      if (now.dayOfTheWeek() == 2)  {                                                   // Dienstag
        if (Safe == 1) {  
          servo.write(Auf);
          digitalWrite(LampeAuto, HIGH);
          ZustandAutomatik = 1;}} 
      
      if (now.dayOfTheWeek() == 3)  {                                                   // Mittwoch
       if (Safe == 1) {
        if (now.hour() == 10 || now.hour() == 11 || now.hour() == 12 || now.hour() == 13 || now.hour() == 14 || now.hour() == 15 || now.hour() == 16 || now.hour() == 17) {
            servo.write(Zu);
            digitalWrite(LampeAuto, LOW);
            ZustandAutomatik = 0;}
          else {
            servo.write(Auf);
            digitalWrite(LampeAuto, HIGH);
            ZustandAutomatik = 1;}}}

       if (now.dayOfTheWeek() == 4)  {                                                  // Donnerstag
        if (Safe == 1) {
          if (now.hour() == 15 || now.hour() == 16 || now.hour() == 17) {
            servo.write(Zu);
            digitalWrite(LampeAuto, LOW);
            ZustandAutomatik = 0;}
          else {
            servo.write(Auf);
            digitalWrite(LampeAuto, HIGH);
            ZustandAutomatik = 1;}}}
          
       if (now.dayOfTheWeek() == 5)  {                                                   // Freitag
        if (Safe == 1) {
          if (now.hour() == 10 || now.hour() == 11 || now.hour() == 12 || now.hour() == 15 || now.hour() == 16 || now.hour() == 17 || now.hour() == 18) {
            servo.write(Zu);
            digitalWrite(LampeAuto, LOW);
            ZustandAutomatik = 0;}
          else {
            servo.write(Auf);
            digitalWrite(LampeAuto, HIGH);
            ZustandAutomatik = 1;}}}
          
      if (now.dayOfTheWeek() == 6)  {                                                   // Samstag
       if (Safe == 1) {
        servo.write(Auf);
        digitalWrite(LampeAuto, HIGH);
        ZustandAutomatik = 1;}}

    else {digitalWrite(LampeAutoIO, LOW);}                                              
    }
  
  if (client) {                                                                         // Wenn Client gefunden, dann...    
    
    Serial.println(F("Neuer Client verfügbar"));
    
    while (client.connected()) {
     
      if (client.available()) {
        
        char X = client.read();                                                         // Speichert die ankommenden Daten (jeweils nur ein Zeichen) im Zeichen (8bit) "X"
        int Klappe = digitalRead(SensorServo);                                          // Sensor Wert wird im Moment der HTTP Abfrage ausgelesen (muss nicht kontinuierlich laufen) 
        DateTime now = RTC.now();                                                       // Liest aktuelle Uhrzeit im Moment der HTTP Abfrage aus
        
        if (readString.length() < 100) {                                                // Speichert den ganzen Request im String bis zu einer Länge von 100
          readString += X;}

        if (X == '\n') {                                                                // Wenn eine leere Zeile ankommt (Signalisiert das Ende eines HTTP Request --> nicht ganz optimal, da ja auch davor leere Zeilen vorkommen)
                           
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
          
          client.print("<a href=\"/?1auto\"\">Automatische</a>");                        // href Verlinkung um Aktion auszuführen 
          client.print(" oder ");
          client.print("<a href=\"/?1manu\"\">Manuelle</a>");                            // href Verlinkung um Aktion auszuführen
          client.print(" Steuerung<br /><br />");
          
          if (Automatik == 0) {
            client.println("<a href=\"/?1auf\"\">Klappe Auf</a>");                       // href Verlinkung um Aktion auszuführen 
            client.print(" oder ");
            client.println("<a href=\"/?1zu\"\">Klappe Zu</a><br /><br />");}            // href Verlinkung um Aktion auszuführen

          client.println(F("Status Automatikmodus: Klappe ist "));                       // Zeigt an ob die Automatik die Klappe öffnen oder sperren würde
            if (ZustandAutomatik == 1) {
              client.println("offen");}
            if (ZustandAutomatik == 0) {
              client.println("zu");}
          client.println("<br /><br />");

          client.println(F("Status aktuell: Klappe ist "));                              // Gibt den aktuellen Zustand der Klappe aus, basierend auf dem Sensor Wert
            if (Klappe == 1) {
              client.println("offen");
              digitalWrite(LampeServo, LOW);}
            if (Klappe == 0) {
              client.println("zu");
              digitalWrite(LampeServo, HIGH);}
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
              if (digitalRead(SensorKlappe) == 1) { 
               servo.write(Auf);
               Serial.println(F("Klappe Auf"));}}
               
          if (readString.indexOf("?1zu") > 0){                                           // Servo Zufahren, anhand von href im ankommenden HTTP String
              if (digitalRead(SensorKlappe) == 1) { 
               servo.write(Zu);
               Serial.println(F("Klappe Zu"));}}  
                 
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
