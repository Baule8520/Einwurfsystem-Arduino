# Einwurfsystem
Aufbau einer elektrischen Steuerung für ein Einwurfsystem, beispielsweise einer Rückgabeklappe für Bücher.

Die Software wurde von mir schrittweise zusammengebaut. Ich habe Sie in 5 Teilbereiche zerlegt, um die jeweilige Funktion der Komponente genau verstehen zu können: 

1. Ethernet Funktionalität herstellen (Ethernet_)
2. WebServer Funktionen testen (WebServer_)
3. Real Time Clock (RTC_)
4. Schrittmotor Ansteuerung (Schrittmotor_)
5. Der sogenannte Master, also das Hauptprogramm, in dem alles zusammen läuft (Master_)

Die verwendete Hardware war ein Arduino UNO mit aufgestecktem Ethernet Shield Version 2.
Dementsprechend muss die Arduino IDE zum Kompilieren und Uploaden installiert sein.

Anbei sind außerdem die von mir verwendeten Libraries (libraries)

Ausführliche Dokumentation unter: https://wiki.tum.de/pages/viewpage.action?pageId=394625561
