

Für die WiFi-Clock wurden eine Bibliotheken erstellt (Own_Libraries), um mit der angebundenen Peripherie zu
interagieren.

Im Zusammenhang mit der WiFi-Clock wurden folgende Bibliotheken entwickelt.

rotary.h				Dient für die Interaktion mit dem Encoder
button.h				Dient für die Interaktion mit dem On-Board-Button
serialprotocol.h		Dient für die Interaktion mit der internen Uhr und dem Rotary mit UART als Schnittstelle
wifiprotocol.h			Dient für die Nachrichtenübertragung zwischen 2 Mikrocontroller



Deteils zur genauen Verwendung und Beschreibung der Bibliothek sind in der MyLibrary/Inc/mylib_serialprot.c Datei
Folgend ist die Nutzung für Eingabe und Ausgaben mit Putty beschrieben
	
					
  ==============================================================================
                          ##### Start #####
  ==============================================================================					
					
Nach dem Upload auf den STM32 und den anschließenden start von Putty kann sofort das jeweilige Kommando eingegeben werden.				

  ==============================================================================
                          ##### Syntax - Eingabe #####
  ==============================================================================

##### Kommandosyntax #####

<SoF><Kommandoname><Kommandokind><Parameter1>,<Parameter2>...<EoF> */

Beispiel:
#time_hours?\r		gibt die Stunden der Uhrzeit zurück
#time_hours=11\r	setzt die Stunden der Uhrzeit
#befehlsname,Parameter1:Parameter2\r

Der Befehlname besteht aus Buchstaben
Die Parameter können je nach Befehl eine unterschiedliche Anzahl besitzen
Die Bestätigung/Absenden des Befehls erfolgt mit der Enter-Taste(\r)

##### Beschreibung #####

SoF(Zeichen für den Befehlsbeginn)	 						#
CMD(Befehle bestehen aus 3 Buchstaben)						time_hours
CMD_KIND(Set, Get oder Execute- Befehll)					=
PARAMETER1 (Zahl oder Text)									11
SEP(Trennzeichen zwischen den Parametern) 					,
EoF(Zeichen für das Begehlsende) 							\r    (drücken der Enter-Taste)

##### Mögliche Befehle #####

#time?														aktuelle Uhrzeit ausgeben
#time_hours?												aktuelle Stunden der Uhrzeit ausgeben
#time_hours=[0-23]											Stunden der Uhrzeit setzen
#time_minutes?												aktuelle Minuten der Uhrzeit ausgeben
#time_minutes=[0-59]										Minuten der Uhrzeit setzen
#time_seconds?												aktuelle Sekunden der Uhrzeit ausgeben
#time_seconds=[0-59]										Sekunden der Uhrzeit setzen
#alarm?														aktuelle Alarmzeit ausgeben
#alarm_hours?												aktuelle Stunden der Alarmzeit ausgeben
#alarm_hours=[0-23]											Stunden der Alarmzeit setzen
#alarm_minutes?												aktuelle Minuten der Alarmzeit ausgeben
#alarm_minutes=[0-59]										Minuten der Alarmzeit setzen
#alarm_seconds?												aktuelle Sekunden der Alarmzeit ausgeben
#alarm_seconds=[0-59]										Sekunden der Alarmzeit setzen
#alarm_state?												Alarmstatus ausgeben
#alarm_state=[0-1]											Alarm Ein[1] oder Aus[0] -Schalten		
#alarm_quit													Alarm quittieren


*-- Overflow --*
Sollten mehr als 20 Zeichen eingegeben worden sein, so ist eine Neueingabe erforderlich, da dies kein
Befehlt mehr sein kann.
Es wird folgendes ausgegeben								-> OV						


  ==============================================================================
                          ##### Syntax - Ausgabe #####
  ==============================================================================

*-- FALSCHEINGABEN --*
Existiert der Befehl nicht oder der Befehl 
wurde falsch Eingegeben wird folgendes Ausgegeben:			STM32-NACK -> "letzte Eingabe"

*-- KORREKTE EINGABEN --*
Werden bei Korrektheit bestätigt durch: 					STM32-ACK -> "letzte Eingabe"

*-- ERWARTUNG EINES ERGEBISSES --*
Wird ein Befehl eingegeben welcher zur Berechnungen 
dient erfolgt zusätzlich das Ergebnis: 						STM32-ACK -> "letzte Eingabe" => ERGEBNIS\r






