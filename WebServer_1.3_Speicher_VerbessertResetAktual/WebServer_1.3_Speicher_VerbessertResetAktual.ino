/*
  WebServer zur Steuerung eines Servos, Abfragen eines Sensors und Speicherung von eingehenden Sensor Daten

  By Paul Zech

  Version 1.1
 
 */


#include <SPI.h>                                            // SPI Controller
#include <Ethernet.h>                                       // Ethernet Funktionalität
#include <Servo.h>                                          // Servo Funktion


byte mac[] = {0xA8, 0x61, 0x0A, 0xAE, 0x42, 0xD7};          // MAC Adresse des Ethernet Shield hier eintragen
IPAddress ip(192, 168, 188, 10);                            // Freie IP-Adresse hier eintragen
EthernetServer server(80);                                  // Ethernet Port hier eintragen, für HTTP ist 80 der Standardport
String readString;                                          // Erstellen eines String in dem die Kommunikation mit dem Client gespeichert wird
Servo servo;                                                // Name des Servo Motors
int SensorKlappe = 6;                                       // Anschluss des Sensors für die Klappe
int SensorEinwurf = 7;                                      // Anschluss des Sensors für die eingeworfenen Gegenstände
int Kontrolllampe = 2;                                      // Anschluss Kontrolllampe
int Klappe = digitalRead(SensorKlappe);                     // Sensor Wert der Klappe initial abspeichern
int Eingeworfen = 0;                                        // Anzahl der eingeworfenen Gegenstände auf 0 setzen
int Einwurf = 1;                                            // Zwischenspeicher 1 zum Entprellen der Taste der eingeworfenen Gegenstände
int Prello = 0;                                             // Zwischenspeicher 2 zum Entprellen der Taste der eingeworfenen Gegenstände


void setup() {

  servo.attach(5);                                          // Anschluss des Servos
  pinMode(SensorKlappe, INPUT_PULLUP);                      // Sensor als Input mit Pull Up Widerstand klassifizieren
  pinMode(SensorEinwurf, INPUT_PULLUP);
  pinMode(Kontrolllampe, OUTPUT);

  Serial.begin(9600);                                       // Seriellen Monitor Starten --> Debugging
  while (!Serial) { ; }
  
  Serial.println("WebServer Version 1.2\n");                // Titel des Programms ausgeben

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
  Serial.print("\n");
}


void loop() {                                                                             

  EthernetClient client = server.available();                            // Nach neuem Client suchen
    
  int Einwurf = digitalRead(SensorEinwurf);                              // Liest den Sensor kontinuierlich ein (Pull Up: Standartsensorwert = 1)
   
    if (Einwurf == 0 && Prello == 0) {                                   // Zählt bei Einwurf == 0 den Eingeworfen nach oben
              Eingeworfen += 1;
              Einwurf = 1;
              Prello = 1;}
    if (Einwurf == 1 && Prello == 1) {                                   // Entprellt die Taste um eine Viertelsekunde
              delay(500);
              Prello = 0;}
  
   if (client) {                                                         // Wenn Client gefunden, dann...    
    
    Serial.println("Neuer Client verfügbar");
    
    while (client.connected()) {
     
      if (client.available()) {
        
        char X = client.read();                                          // Speichert die ankommenden Daten (jeweils nur ein Zeichen) im Zeichen (8bit) "X"
        int Klappe = digitalRead(SensorKlappe);                          // Sensor Wert wird im Moment der HTTP Abfrage ausgelesen (muss nicht kontinuierlich laufen) 
        
        if (readString.length() < 100) {                                 // Speichert den ganzen Request im String bis zu einer Länge von 100
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
          client.println("<h1>Test - Steuerung Einwurfkasten + Datenerfassung</h1>");
          client.println("<br />");
          client.println("<br />");
          client.println("<br />");
          client.println("<a href=\"/?1auf\"\">Klappe Auf</a>");         // href Verlinkung um Aktion auszuführen 
          client.println("<br />");
          client.println("<br />");
          client.println("<a href=\"/?1zu\"\">Klappe Zu</a><br />");     // href Verlinkung um Aktion auszuführen
          client.println("<br />");
          client.println("<br />");
          client.println("Status: Klappe ist ");                         // Gibt den aktuellen Zustand der Klappe aus, basierend auf dem Sensor Wert
            if (Klappe == 1) {
              client.println("offen");
              digitalWrite(Kontrolllampe, LOW);}
            if (Klappe == 0) {
              client.println("zu");
              digitalWrite(Kontrolllampe, HIGH);}
          client.println("<br />");
          client.println("<br />");
          client.println("Eingeworfene Gegenstaende: ");                 // Gibt die Anzahl von "Eingeworfen" aus
          client.println(Eingeworfen);
          client.println("<br />");
          client.println("<br />");
          client.println("<a href=\"/?1reset\"\">Reset des Zaehlers</a><br />");         
          client.println("<br />");
          client.println("<br />");
          client.println("<a href=\"/?1aktual\"\">Aktualisieren</a>");    
          client.println("</body>");
          client.println("</html>");
          
          Serial.println("Daten abgesendet");                            // Information über erfolgreiche Sendung am seriellen Monitor

          delay(2);                                                      // Warten bis Client Daten empfangen hat
    
          client.stop();                                                 // Verbindung beenden

          if (readString.indexOf("?1auf") > 0){                          // Servo auffahren, anhand von href im ankommenden HTTP String
               servo.write(40);
               Serial.println("Klappe Auf");
           }
          if (readString.indexOf("?1zu") > 0){                           // Servo Zufahren, anhand von href im ankommenden HTTP String
                servo.write(170);
               Serial.println("Klappe Zu");
           }    
          if (readString.indexOf("?1reset") > 0){                        // Reset der eingeworfenen Gegenstände
                Eingeworfen = 0;
               Serial.println("Reset");
           }
            
          readString = "";                                               // String entleeren
          
          }
      }
    }
    
    Serial.println("Verbindung geschlossen\n\n");
    
  }
}
