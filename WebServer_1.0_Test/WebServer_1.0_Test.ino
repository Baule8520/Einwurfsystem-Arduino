/*
  WebServer zur Steuerung eines Servos und Abfragen eines Sensors

  By Paul Zech

  Version 1.0
 
 */


#include <SPI.h>                                            // SPI Controller
#include <Ethernet.h>                                       // Ethernet Funktionalität
#include <Servo.h>                                          // Servo Funktion


byte mac[] = {0xA8, 0x61, 0x0A, 0xAE, 0x42, 0xD7};          // MAC Adresse des Ethernet Shield hier eintragen
IPAddress ip(192, 168, 188, 10);                            // Freie IP-Adresse hier eintragen
EthernetServer server(80);                                  // Ethernet Port hier eintragen, für HTTP ist 80 der Standardport
String readString;                                          // Erstellen eines String in dem die Kommunikation mit dem Client gespeichert wird
Servo servo;                                                // Name des Servo Motors
int Sensor = 6;                                             // Anschluss des Sensors
int SensorWert = digitalRead(Sensor);                       // Sensor Wert initial abspeichern in der Variablen SensorWert

void setup() {

  servo.attach(5);                                          // Anschluss des Servos
  pinMode(Sensor, INPUT);                                   // Sensor als Input klassifizieren

  Serial.begin(9600);                                       // Seriellen Monitor Starten --> Debugging
  while (!Serial) { ; }
  
  Serial.println("WebServer Version 1.0\n");                // Titel des Programms ausgeben

  Ethernet.begin(mac, ip);                                  // Start des Ethernet Zugriffs

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {    // Fehlermeldung: Keine Hardware
    Serial.println("Ethernet Shield nicht aufgesteckt");
    while (true) {
      delay(1); }}
  if (Ethernet.linkStatus() == LinkOFF) {                   // Fehlermeldung: Kein Kabel
    Serial.println("Kein Netzwerkkabel eingesteckt");}
  
  server.begin();                                           // Start des Servers
  Serial.print("Server befindet sich auf ");
  Serial.println(Ethernet.localIP());                       // IP Adresse des Servers ausgeben
}


void loop() {                                                                             
  
  EthernetClient client = server.available();                            // Nach neuem Client suchen
  
  if (client) {                                                          // Wenn Client gefunden, dann...    
    
    Serial.println("Neuer Client verfügbar");
    
    while (client.connected()) {
     
      if (client.available()) {
        
        char X = client.read();                                          // Speichert die ankommenden Daten (jeweils nur ein Zeichen) im Zeichen (8bit) "X"
        int SensorWert = digitalRead(Sensor);                            // Sensor Wert wird immer wieder ausgelesen
        
        if (readString.length() < 100) {                                 // Speichert den ganzen Request bis im String bis zu einer Länge von 100
          readString += X;
          }

        if (X == '\n') {                                                 // Wenn eine leere Zeile ankommt (Signalisiert das Ende eines HTTP Request --> nicht ganz optimal, da ja auch davor leere Zeilen vorkommen)
                           
          client.println("HTTP/1.1 200 OK");                             // Inhalt der "Website" --> Header
          client.println("Content-Type: text/html");
          client.println();
          
          client.println("<!DOCTYPE HTML>");                             // Inhalt der "Website" --> Content
          client.println("<html>");
          client.println("<head>");
          client.println("<title>Steuerung</title>");
          client.println("</head>");
          client.println("<body>");
          client.println("<h1>Test - Steuerung Einwurfkasten</h1>");
          client.println("<br />");
          client.println("<br />");
          client.println("<br />");
          client.println("<a href=\"/?1auf\"\">Klappe Auf</a>");   
          client.println("<br />");
          client.println("<br />");
          client.println("<a href=\"/?1zu\"\">Klappe Zu</a><br />"); 
          client.println("<br />");
          client.println("<br />");
          client.println("Status: Klappe ist ");                         // Gibt den aktuellen Zustand der Klappe aus, basierend auf dem Sensor Wert
            if (SensorWert == 1) {
              client.println("offen");}
            if (SensorWert == 0) {
              client.println("zu");}
          client.println("</body>");
          client.println("</html>");
          
          Serial.println("Daten abgesendet");                            // Information über erfolgreiche Sendung am seriellen Monitor

          delay(2);                                                      // Warten bis Client Daten empfangen hat
    
          client.stop();                                                 // Verbindung beenden

          if (readString.indexOf("?1auf") > 0){                          // Servo auffahren
               servo.write(40);
               Serial.println("Klappe Auf");
           }
           if (readString.indexOf("?1zu") > 0){                          // Servo Zufahren
                servo.write(170);
               Serial.println("Klappe Zu");
           }    
           
          readString = "";                                               // String entleeren
          
          }
      }
    }
    
    Serial.println("Verbindung geschlossen\n\n");
    
  }
}
