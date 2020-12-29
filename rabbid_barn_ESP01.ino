/*
 Name:		rabbid_barn_ESP01.ino
 Created:	2020-05-05 10:02:39
 Author:	z003pr4u
*/


#include <WebSocketsServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h> 
#include <FS.h>
//#define test
#ifdef test
#define LOG(x) Serial.println(x);
IPAddress WebInt(192, 168, 5, 1);
IPAddress WebGetaway(192, 168, 5, 1);
IPAddress WebSubnet(255, 255, 255, 0);
IPAddress TCP_Server(192, 168, 4, 1);
IPAddress TCP_Gateway(192, 168, 4, 1);
IPAddress TCP_Subnet(255, 255, 255, 0);
IPAddress Own(192, 168, 4, 102);
const String  Devicename = "2";//device name for ESP for refrigerator
#else
#define LOG(x);
IPAddress WebInt(192, 168, 5, 1);
IPAddress WebGetaway(192, 168, 5, 1);
IPAddress WebSubnet(255, 255, 255, 0);
IPAddress TCP_Server(192, 168, 4, 1);
IPAddress TCP_Gateway(192, 168, 4, 1);
IPAddress TCP_Subnet(255, 255, 255, 0);
IPAddress Own(192, 168, 4, 101);
const String  Devicename = "1";//device name for ESP for refrigerator
#endif
#define TCPPort_Web_Client 80
#define TCPPort_Websocket 81
#define  TCPPort 2390

const char* ssid = "DataTransfer";
const char* password = "BelovSer";//"DataTransfer", "Belov"

const char* web_ssid = "RabbitBarn";
const char* web_password = "Belov_628";


WiFiClient    TCP_Client;
WebSocketsServer webSocket = WebSocketsServer(TCPPort_Websocket);
WiFiServer  TCP_SERVER(TCPPort);
ESP8266WebServer server(TCPPort_Web_Client);
#include "web&file_setup.h"
String temp, humid, status, air;
namespace dataUNO {
	namespace {
		byte nibble(char c)
		{
			if (c >= '0' && c <= '9')
				return c - '0';

			if (c >= 'a' && c <= 'f')
				return c - 'a' + 10;

			if (c >= 'A' && c <= 'F')
				return c - 'A' + 10;

			return 0;  // Not a valid hexadecimal character
		}
		byte getByte(String data) {
			byte state = B00000000;
			if (data.length() > 1) {
				state = nibble(data[0]) << 4;
				state |= nibble(data[1]);
			}
			else state |= nibble(data[0]);
			return state;
		}
	}
	/*float temp;
	float humid;
	int humidAwer;
	byte state;
	String timeLeft;
	String timeSet;*/

	void parseInput(String data) { /*("{" + String(DHTInDoor.tempBuf) + "," + String(DHTInDoor.humidBuf) + "," +
		String(DHTOutDoor.tempBuf) + "," + String(DHTOutDoor.humidBuf) + "," + String(sun.valueBuf) + "," +
			String(air.valueBuf) + "," + dataString + "," + String(mq135test.valueBuf) + "," + String(mq9test.valueBuf) + "}"*/
		//temp, humid, status, air
		LOG(data)
			webSocket.broadcastTXT(data);
		int beginning = data.indexOf('{') + 1;
		int fullEnd = data.indexOf('}');
		if (beginning != 0 && fullEnd != 0) {
			int currentEnd = data.indexOf(',', beginning);
			temp = data.substring(beginning, currentEnd);
			
			beginning = currentEnd + 1;
			currentEnd = data.indexOf(',', beginning);
			humid = data.substring(beginning, currentEnd);
			for (int i = 0; i < 4; i++) {
				beginning = currentEnd + 1;
				currentEnd = data.indexOf(',', beginning);
			}
			
			air = data.substring(beginning, currentEnd);
			
			beginning = currentEnd + 1;
			currentEnd = data.indexOf(',', beginning);
			status = data.substring(beginning, currentEnd);
			LOG(temp+"  "+humid+"  "+ air + "   " + status)
			

			/*if (get_field_valueString(message, "temp:", &value)) temp = value;
		if (get_field_valueString(message, "humid:", &value)) humid = value;
		if (get_field_valueString(message, "status:", &value)) status = value;
		if (get_field_valueString(message, "air:", &value)) air = value;*/

			/*Serial.println(dataUNO::state, BIN);
			if (bitRead(dataUNO::state, 0))LOG("deodorantActivated=true")
			else LOG("deodorantActivated=false")
			if (bitRead(dataUNO::state, 1))LOG("humanBodyDeteckted=true")
			else LOG("humanBodyDeteckted=false")
			if (bitRead(dataUNO::state, 2))LOG("buttonStart=true")
			else LOG("buttonStart=false")
			if (bitRead(dataUNO::state, 3))LOG("fanWork=true")
			else LOG("fanWork=false")
			if (bitRead(dataUNO::state, 4))LOG("webStart=true")
			else LOG("webStart=false")
			if (bitRead(dataUNO::state, 5))LOG("humidStart=true")
			else LOG("humidStart=false")
			if (bitRead(dataUNO::state, 6))LOG("coutnFull=true")
			else LOG("coutnFull=false")
			if (bitRead(dataUNO::state, 7))LOG("light=true")
			else LOG("light=false")*/
		}
	}

};

bool connectionTried = false;
bool  connectionEstablished = false;
unsigned long timeConnectioTried, timeAttemptToConnect, checkconnection;
String fieldsInLogMes;
unsigned long UNIXtime;
unsigned long actualtime;
unsigned long lastNTPResponse;

unsigned long lastUpdateTime = 0; // Track the last update time
const unsigned long postingInterval = 5L * 1000L; // s
// const unsigned long updateInterval = 15L * 1000L; // Update once every 15 seconds

class TimeSet {
	int monthDays[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 }; // API starts months from 1, this array starts from 0
	int *shift;
	time_t lastUpdate;//time when the time was updated, used for current time calculation
	time_t actual_Time;
	time_t UNIXtime;
public:
	int NowMonth;
	int NowYear;
	int NowDay;
	int NowWeekDay;
	int lastShift;
	int NowHour;
	int NowMin;
	int NowSec;
	int Now10Min;
	String FileNewManeParameter; //defines file name assingnement whether: min, 10min, hour, day
	int sec() {
		updateCurrecnt();
		return actual_Time % 60;
	};
	int min() {
		updateCurrecnt();
		return actual_Time / 60 % 60;
	};
	int hour() {
		updateCurrecnt();
		return actual_Time / 3600 % 24;
	};

	time_t SetCurrentTime(time_t timeToSet) { //call each time after recieving updated time
		lastUpdate = millis();
		UNIXtime = timeToSet;
	}//call each time after recieving updated time
/*	void updateDay() {
		updateCurrecnt();
		breakTime(&NowYear, &NowMonth, &NowDay, &NowWeekDay);
		Serial.println(String(NowYear) + ":" + String(NowMonth) + ":" + String(NowDay) + ":" + String(NowWeekDay));
	}// call in setup and each time when new day comes
	bool Shift() {  //check whether there is a new day comes, first call causes alignement of tracked and checked day
		//Serial.println("lastShift " + String(lastShift) + "  *shift " + String(*shift));
		if (lastShift != *shift) {
			lastShift = *shift;
			return true;
		}
		else return false;
	} //call to check whether the day is changed
	void begin(int shiftType = 1) //1-day,2-hiur,3-10 minutes
	{
		switch (shiftType)
		{
		case 1:shift = &NowDay;
			FileNewManeParameter = "day";
			break;
		case 2:shift = &NowHour;
			FileNewManeParameter = "hour";
			break;
		case 3:shift = &Now10Min;
			FileNewManeParameter = "10min";
			break;
		case 4:shift = &NowMin;
			FileNewManeParameter = "min";
			break;
		default:shift = &NowDay;
			FileNewManeParameter = "day";
			break;
		}

	};*/
private:

	void updateCurrecnt() {
		actual_Time = UNIXtime + (millis() - lastUpdate) / 1000;

	}

	bool LEAP_YEAR(time_t Y) {
		//((1970 + (Y)) > 0) && !((1970 + (Y)) % 4) && (((1970 + (Y)) % 100) || !((1970 + (Y)) % 400)) ;
		if ((1970 + (Y)) > 0) {
			if ((1970 + (Y)) % 4 == 0) {
				if ((((1970 + (Y)) % 100) != 0) || (((1970 + (Y)) % 400) == 0)) return true;
			}
			else return false;
		}
		else return false;
	}

	void breakTime(int *Year, int *Month, int *day, int *week) {
		// break the given time_t into time components
		// this is a more compact version of the C library localtime function
		// note that year is offset from 1970 !!!

		int year;
		int month, monthLength;
		uint32_t time;
		unsigned long days;

		time = (uint32_t)actual_Time;
		NowSec = time % 60;
		time /= 60; // now it is minutes
		NowMin = time % 60;
		Now10Min = time % 600;
		time /= 60; // now it is hours
		NowHour = time % 24;
		time /= 24; // now it is days
		*week = int(((time + 4) % 7));  // Monday is day 1 

		year = 0;
		days = 0;
		while ((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
			year++;
		}
		*Year = year + 1970; // year is offset from 1970 

		days -= LEAP_YEAR(year) ? 366 : 365;
		time -= days; // now it is days in this year, starting at 0

		days = 0;
		month = 0;
		monthLength = 0;
		for (month = 0; month < 12; month++) {
			if (month == 1) { // february
				if (LEAP_YEAR(year)) {
					monthLength = 29;
				}
				else {
					monthLength = 28;
				}
			}
			else {
				monthLength = monthDays[month];
			}

			if (time >= monthLength) {
				time -= monthLength;
			}
			else {
				break;
			}
		}
		*Month = month + 1;  // jan is month 1  
		*day = time + 1;     // day of month
	}
}Time_set;
class task {
public:
	unsigned long period;
	bool ignor = false;
	void reLoop() {
		taskLoop = millis();
	};
	bool check() {
		if (!ignor) {
			if (millis() - taskLoop > period) {
				taskLoop = millis();
				return true;
			}
		}
		return false;
	}
	void StartLoop(unsigned long shift) {
		taskLoop = millis() + shift;
	}
	task(unsigned long t) {
		period = t;
	}
private:
	unsigned long taskLoop;

};
task task_sendUpdatToServer(6000);
task taskSendConf2UNO(50000);
task task_getLogStringFromUNO(2000);
task askTimeTask(20000);
task task_sendTimeToUno(60000);
task task_checkClient(5000);
void setup() {
	Serial.begin(9600);
	WiFi.mode(WIFI_AP_STA);//WIFI_STA WIFI_AP_STA 
	setupWeb();
	setupClient();
	//server.on("/", HTTP_GET, handleFileRead);
	server.onNotFound([]() {                              // If the client requests any URI
		if (!handleFileRead(server.uri()))                  // send it if it exists
			server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
	});
	server.begin();                           // Actually start the server
	webSocket.begin();                          // start the websocket server
	startSPIFFS();
	webSocket.onEvent(webSocketEvent);          // if there's an incomming websocket message, go to function 'webSocketEvent'
}
void webSocketEvent(byte num, WStype_t type, uint8_t* payload, size_t length)
{
	if (length) {
		
		String messageSt;
		for (int i = 0; i < length; i++) messageSt += (char)payload[i];
		LOG("websocket got:" + messageSt);
		Serial.println(messageSt);
		
	}
}

void Send_Request_To_Server(String dataToSend) {
	TCP_Client.setNoDelay(1);
	TCP_Client.println(dataToSend);
	//Serial.println(dataToSend);
}

//====================================================================================

//void Check_WiFi_and_Connect() {
//	//Serial.println("Not Connected...trying to connect...");
//	yield();
//	WiFi.mode(WIFI_STA);                                // station (Client) Only - to avoid broadcasting an SSID ??
//	//WiFi.begin(ssid, password,0);                    // the SSID that we want to connect to
//	WiFi.begin(ssid, password);
//	//Serial.println("ssid: " + String(ssid) + "  password:" + String(password));
//}

//void Print_connection_status() {
//
//	// Printing IP Address --------------------------------------------------
//	//Serial.println("Connected To      : " + String(WiFi.SSID()));
//	//Serial.println("Signal Strenght   : " + String(WiFi.RSSI()) + " dBm");
//	//Serial.print("Server IP Address : ");
//	//Serial.println(TCP_Server);
//	//Serial.print("Device IP Address : ");
//	//Serial.println(WiFi.localIP());
//
//	// conecting as a client -------------------------------------
//	Tell_Server_we_are_there();
//}

//====================================================================================



//====================================================================================

void loop() {
	if (task_checkClient.check()) {
		if (TCP_SERVER.hasClient()) {
			WiFiClient Temp = TCP_SERVER.available();
			IPAddress IP = Temp.remoteIP();
			LOG("Conneted client ");
			LOG(IP);
			task_checkClient.ignor = true;
		}
	}


	if (task_getLogStringFromUNO.ignor) {
		Check_WiFi_and_Connect_or_Reconnect();          // Checking For Connection after recieving logstring from UNO
	}
	if (task_getLogStringFromUNO.check()) Serial.println("getLogString:;");

	yield();
	if (TCP_Client.connected()) {

		if (TCP_Client.available() > 0) {                     // Check For Reply
			String line = TCP_Client.readStringUntil('\r');     // if '\r' is found
			if (!processMessageTCP(line));
				Serial.println(line);
		}
		if (task_sendUpdatToServer.check())Send_Request_To_Server("Device:" + Devicename + ";time:" + String(millis()) +
			";signal:" + String(WiFi.RSSI()) +
			";temp:" + temp + ";humid:" +
			humid + ";air:"+air + ";status:" + status + ";");

	}
	if (Serial.available()) ReadDataSerial();
	if (!TCP_Client.connected())  Tell_Server_we_are_there();
	if (taskSendConf2UNO.check()) {
		if (TCP_Client.connected()) Serial.println("connected:;");
	}
//====================================
	if (askTimeTask.check()) askTime();
	actualtime = UNIXtime + (millis() - lastNTPResponse) / 1000;
	if (task_sendTimeToUno.check()) Serial.println("hour:" + String(Time_set.hour()) + ";min:"+String( Time_set.min()) + ";");
	server.handleClient();                    // Listen for HTTP requests from clients
	webSocket.loop();
	yield();
}
//==================================================
void askTime() {

	if (TCP_Client.connected()) {
		TCP_Client.setNoDelay(1);
		TCP_Client.println("Device:" + Devicename + ";get:1;");
		
		
		
		
		
		
		
		Serial.println("Device:" + Devicename + ";get:1;");
	}
}


//void Check_WiFi_and_Connect_or_Reconnect() {
//	if (WiFi.status() != WL_CONNECTED) {
//
//		TCP_Client.stop();                                  //Make Sure Everything Is Reset
//		WiFi.disconnect();
//		//Serial.println("Not Connected...trying to connect...");
//		//delay(50);
//		WiFi.mode(WIFI_STA);                                // station (Client) Only - to avoid broadcasting an SSID ??
//		WiFi.begin(ssid, password);                         // the SSID that we want to connect to
//		for (int i = 0; i < 10; ++i) {
//			if (WiFi.status() != WL_CONNECTED) {
//				delay(500);
//				//Serial.print(".");
//			}
//			else {
//				//Serial.println("Connected To      : " + String(WiFi.SSID()));
//				//Serial.println("Signal Strenght   : " + String(WiFi.RSSI()) + " dBm");
//				//Serial.print("Server IP Address : ");
//				//Serial.println(TCP_Server);
//				//Serial.print("Device IP Address : ");
//				//Serial.println(WiFi.localIP());
//				// conecting as a client -------------------------------------
//				Tell_Server_we_are_there();
//				break;
//			}
//
//		}
//	}
//}

bool get_field_value_UL(String Message, String field, unsigned long* value, int* index) {
	int fieldBegin = Message.indexOf(field) + field.length();
	int check_field = Message.indexOf(field);
	int ii = 0;
	*value = 0;
	*index = 0;
	bool indFloat = false;
	if (check_field != -1) {
		int filedEnd = Message.indexOf(';', fieldBegin);
		if (filedEnd == -1) { return false; }
		int i = 1;
		char ch = Message[filedEnd - i];
		while (ch != ' ' && ch != ':') {
			if (isDigit(ch)) {
				int val = ch - 48;
				if (!indFloat)ii = i - 1;
				else ii = i - 2;
				*value = *value + ((val * pow(10, ii)));
			}
			else if (ch == '.') { *index = i - 1; indFloat = true; }
			i++;
			if (i > (filedEnd - fieldBegin + 1) || i > 10)break;
			ch = Message[filedEnd - i];
		}

	}
	else return false;
	return true;
}

void ReadDataSerial() { //reads data from ESP serial and checks for validity then sens responce by G:xxx], if gets re:from100 to 299 than LEDOK ON
	
	String value;
	String message;
	int i = 0;
	while (Serial.available()) {
		message = Serial.readStringUntil('\r');
		if (!task_getLogStringFromUNO.ignor) {
			if (get_field_valueString(message, "Device:1;get:3;", &value)) {
				fieldsInLogMes = message;
				TCP_Client.println(fieldsInLogMes);
				task_getLogStringFromUNO.ignor = true;
			}
		}
		if (get_field_valueString(message, "Device:", &value) && TCP_Client.connected()) {
			Send_Request_To_Server(message);
			return;
		}
		else if (message.indexOf("{")!= -1) dataUNO::parseInput(Serial.readStringUntil('\r'));
		/*if (get_field_valueString(message, "temp:", &value)) temp = value;
		if (get_field_valueString(message, "humid:", &value)) humid = value;
		if (get_field_valueString(message, "status:", &value)) status = value;
		if (get_field_valueString(message, "air:", &value)) air = value;*/

		
	}
}
bool processMessageTCP(String message) {
	int  index;
	if (get_field_value_UL(message, "time:", &UNIXtime, &index)) {
		lastNTPResponse = millis();
		askTimeTask.ignor = true;
		Time_set.SetCurrentTime(UNIXtime);
		return true;
	}
	return false;
}
bool get_field_valueString(String Message, String field, String *value) {
	int check_field = Message.indexOf(field);
	if (check_field != -1) {
		int fieldBegin = Message.indexOf(field) + field.length();
		int filedEnd = Message.indexOf(';', fieldBegin);
		*value = Message.substring(fieldBegin, filedEnd);
	}
	else return false;
	return true;
}