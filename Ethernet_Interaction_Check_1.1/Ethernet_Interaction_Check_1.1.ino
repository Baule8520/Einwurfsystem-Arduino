/*
  Ethernet Interaction

  By Paul Zech

  Ohne abwarten des HTTP Request, dafür mit Interaktionsmöglichkeit mit dem Client.
 
 */


#include <SPI.h>                                            // SPI Controller
#include <Ethernet.h>                                       // Ethernet Funktionalität

byte mac[] = {0xA8, 0x61, 0x0A, 0xAE, 0x42, 0xD7};          // MAC Adresse des Ethernet Shield hier eintragen
IPAddress ip(192, 168, 188, 10);                            // Freie IP-Adresse hier eintragen
EthernetServer server(80);                                  // Ethernet Port hier eintragen, für HTTP ist 80 der Standardport
String readString;                                          // Erstellen eines String in dem die Kommunikation mit dem Client gespeichert wird

void setup() {
  
  pinMode(2, OUTPUT);                                       // Eingebaute LED aus Output klassifizieren

  Serial.begin(9600);                                       // Seriellen Monitor Starten --> Debugging
  while (!Serial) { ; }
  
  Serial.println("Starting Ethernet Interaction Check\n");  // Titel des Programms ausgeben

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

        if (readString.length() < 100) {                                 // Speichert den ganzen Request bis im String bis zu einer Länge von 100
          readString += X;
          }

        if (X == '\n') {                                                 // Wenn eine leere Zeile ankommt (Signalisiert das Ende eines HTTP Request --> nicht ganz optimal, da ja auch davor leere Zeilen vorkommen)
                           
          client.println("HTTP/1.1 200 OK");                             // Inhalt der "Website" --> Header
          client.println("Content-Type: text/html");
          client.println();
          
          client.println("<!DOCTYPE HTML>");                             // Inhalt der "Website" --> Content
          client.println("<html>");
          client.println("Ethernet Connection Test Suceeded");
          client.println("<br />");
          client.println("<br />");
            
          client.println("<a href=\"/?1ein\"\">Ein</a>");                // Inhalt Schalter zum Kontrollieren der Onboard LED
          client.println("<a href=\"/?1aus\"\">Aus</a><br />"); 
             
          client.println("</html>");
          
          Serial.println("Daten abgesendet\n");                          // Information über erfolgreiche Sendung am seriellen Monitor

          delay(2);                                                      // Warten bis Client Daten empfangen hat
    
          client.stop();                                                 // Verbindung beenden

          if (readString.indexOf("?1ein") > 0){                          // LED einschalten
               digitalWrite(2, HIGH);
           }
           if (readString.indexOf("?1aus") > 0){                         // LED ausschalten
               digitalWrite(2, LOW);
           }    

          readString = "";                                               // String entleeren
          }
      }
    }
    
    
    Serial.println("Verbindung geschlossen\n\n");
  }
}
