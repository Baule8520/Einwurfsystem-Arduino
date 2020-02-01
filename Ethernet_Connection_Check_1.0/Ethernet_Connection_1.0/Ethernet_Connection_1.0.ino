/*
  Ethernet Connection

  By Paul Zech

  Basierend auf dem Webserver Beispiel der Arduino Bibliothek.
 
 */


#include <SPI.h>                                            // SPI Controller
#include <Ethernet.h>                                       // Ethernet Funktionalität


byte mac[] = {0xA8, 0x61, 0x0A, 0xAE, 0x42, 0xD7};          // MAC Adresse des Ethernet Shield hier eintragen
IPAddress ip(192, 168, 188, 10);                            // Freie IP-Adresse hier eintragen
EthernetServer server(80);                                  // Ethernet Port hier eintragen, für HTTP ist 80 der Standardport


void setup() {

  Serial.begin(9600);                                       // Seriellen Monitor Starten --> Debugging
  while (!Serial) { ; }
  
  Serial.println("Starting Ethernet Connection Check\n");   // Titel des Programms ausgeben

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


void loop() {                                                            // Interaktion mit dem Client (Browser)                  
  
  EthernetClient client = server.available();                            // Nach neuem Client suchen
  
  if (client) {                                                          // Wenn Client gefunden, dann...
    
    boolean HTTPEnd = false;                                             // Definition einer Boolean Variable ob Daten schon zum Server gesendet werden können             
    
    Serial.println("Neuer Client verfügbar");
    Serial.println("HTTP Request des Clients: \n");
    
    while (client.connected()) {
     
      if (client.available()) {
        
        char HTTPRequest = client.read();                                // Speichert die ankommenden Daten Zeichen für Zeichen im Zeichen (8bit) "HTTPRequest"
        Serial.write(HTTPRequest);                                       // Gibt diese Daten am seriellen Monitor aus
        Serial.print(" ");
        
        if (HTTPRequest == '\n' && HTTPEnd) {                            // Wenn als nächstes Zeichen eine neue Zeile ankommt (Ende des HTTP Request) und unsere Variable "true" ist sendet er die Daten an den Ethernet Stack
                          
          client.println("HTTP/1.1 200 OK");                             // Inhalt der "Website" --> HTTP Content
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println("Refresh: 5");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("Ethernet Connection Test Erfolgreich");
          client.println("</html>");
          Serial.println("Daten abgesendet\n");                          // Information über erfolgreiche Sendung am seriellen Monitor
          
          break;                                                         // Beendet die while Schleife
          }
          
        if (HTTPRequest == '\n') {HTTPEnd = true;}                       // Ist das aktuelle Zeichen eine neue Zeile, dann Variable auf "true"
        else if (HTTPRequest != '\r') {HTTPEnd = false;}                 // Wenn kein Zeichen NICHT im aktuellen HTTPRequest ist dann setze auf "false"
      }
    }
    
    delay(10);                                                           // Warten bis Client Daten empfangen hat
    
    client.stop();                                                       // Verbindung beenden
    Serial.println("Verbindung geschlossen\n\n");
  }
}
