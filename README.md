# Air-conditioner-Ir-controller-with-mqtt

Controlling Air Conditioner using MQTT Cloud
OBJECTIVE:
	This project controls the Air Conditioner remotely; the solution is to cool down a place even at a long distance with any phone using MQTT.
Components Required :
Hardware	Uses:
•	NodeMcu(ESP8266)	Integrating Cloud Mqtt With user
•	IR Transmitter	Transmitting IR pulse for control
•	BC547	Increasing the range of IR
•	Power Supply Module	Regulating supply for this circuit

Software	Uses:
•	Arduino IDE	Programming for Esp8266
•	MQTT Server	Cloud 
•	MQTT Dashboard	APP for User interface
•	HeatPump IR Library	IR Libraries compatible to ESP8266


Code:
	
Name	Description
IR_SEND_PIN	The PIN where the IR LED is connected
WIFI_SSID	Your WiFi network name
WIFI_PASS	WiFi password
MQTT_HOST	Mqtt hostname like mqtt.mydomain.com
MQTT_USER	MQTT Username
MQTT_PASS	MQTT Password

	


Working:














Steps:

     Arduino Configuration:
1.	Start New project in Arduino.
2.	Link for that program: https://www.sysrun.io/wp-content/uploads/2017/02/daikin-remote-clean-1.zip 
3.	With Software uploaded(Aduino,MQTT Dashboard), Hardware connected(NodeMCU,IR, Power supply board )and MQTT connection enabled.
4.	Add library By going to Arduino- >Sketch- >Include Library - >Add zip library->import respective library
   1. PubsubClient,
   2. HeatPumpIR
                                3. ESP8266WiFi
   4.Timer  
   5. ESP8266mDNS
   6. DNSServer.h
   7.ESP8266WebServer.h
   8.WiFiManager.h

Access Point Configuration:
1.	when your ESP starts up, it sets it up in Station mode and tries to connect to a previously saved Access Point
2.	if this is unsuccessful (or no previous network saved) it moves the ESP into Access Point mode and spins up a DNS and WebServer (default ip 192.168.4.1)
3.	using any wifi enabled device with a browser (computer, phone, tablet) connect to the newly created Access Point
4.	because of the Captive Portal and the DNS server you will either get a 'Join to network' type of popup or get any domain you try to access redirected to the configuration portal
5.	choose one of the access points scanned, enter password, click save
6.	ESP will try to connect. If successful, it relinquishes control back to your app. If not, reconnect to AP and reconfigure. 
    

MQTT Configuration:
1.	Login with CloudMQTT
   Create new instances->choose name,plan,region ->submit
   Goto your new instances –it have Server,username,Password,port(add to program)
2.	The software will listen for commands on a specific topic configured via the constant MQTT_COMMAND_CHANNEL.
3.	Download MQTT Dashboard (ios,Android)
4.	In MQTT app we have to link with cloudMQTT by giving servername,Username and passwords
5.	The default configuration is “cmnd/daikin/#  .
6.	To set the power on the A/C, you just publish “1” to the topic “cmnd/daikin/power“. 
7.	The MCU will now send the command via IR
8.	The current values of every setting (power, mode, fan etc.) is obtained as JSON to the topic “stat/daikin/”
 {"power":1,"mode":3, "fan":2,"temp":23,"vswing":0,"hswing":0}

Command overview and value mapping:
•	Command	Descr → Value
•	cmnd/daikin/power	POWER_OFF → 0   POWER_ON → 1
•	cmnd/daikin/mode	MODE_AUTO → 1   MODE_HEAT → 2  MODE_COOL → 3     MODE_DRY → 4  MODE_FAN → 5
•	cmnd/daikin/fan	FAN_AUTO → 0  FAN_1 → 1  FAN_2 → 2  FAN_3 → 3  FAN_4 → 4  FAN_5 → 5
•	cmnd/daikin/temp	Numeric value  VDIR_AUTO → 0  HDIR_AUTO → 0
Circuit Diagram:
 
 


App Interface: 
 




Hardware :

 
