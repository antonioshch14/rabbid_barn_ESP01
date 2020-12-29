#pragma once
void startSPIFFS();
String formatBytes(size_t bytes);
bool handleFileRead(String path);
void update_Web();
String getContentType(String filename);
void setupWeb();
void  setupClient();
void Print_connection_status();
void Tell_Server_we_are_there();
bool sendToServer(String message);
void Check_WiFi_and_Connect_or_Reconnect();
void startSPIFFS() { // Start the SPIFFS and list all contents
	SPIFFS.begin();                             // Start the SPI Flash File System (SPIFFS)
	Serial.println("SPIFFS started. Contents:");
	{
		Dir dir = SPIFFS.openDir("/");
		while (dir.next()) {                      // List the file system contents
			String fileName = dir.fileName();
			size_t fileSize = dir.fileSize();
			Serial.printf("\tFS File: %s, size: %s\r\n", fileName.c_str(), formatBytes(fileSize).c_str());
		}
		Serial.printf("\n");
	}
}
String formatBytes(size_t bytes) { // convert sizes in bytes to KB and MB
	if (bytes < 1024) {
		return String(bytes) + "B";
	}
	else if (bytes < (1024 * 1024)) {
		return String(bytes / 1024.0) + "KB";
	}
	else if (bytes < (1024 * 1024 * 1024)) {
		return String(bytes / 1024.0 / 1024.0) + "MB";
	}
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
	LOG(Serial.println("handleFileRead: " + path));
	if (path.endsWith("/")) path += "HTMLPage1.html";         // If a folder is requested, send the index file
	String contentType = getContentType(path);            // Get the MIME type
	if (SPIFFS.exists(path)) {                            // If the file exists
		File file = SPIFFS.open(path, "r");                 // Open it
		size_t sent = server.streamFile(file, contentType); // And send it to the client
		file.close();                                       // Then close the file again
		return true;
	}
	LOG(Serial.println("\tFile Not Found"));
	return false;                                         // If the file doesn't exist, return false
}
void update_Web() {
	//if (digitalRead(LED)) webSocket.broadcastTXT("L_ON");
	//else 	webSocket.broadcastTXT("L_OFF");
	//LOG("Web updated");
}
String getContentType(String filename) { // determine the filetype of a given filename, based on the extension
	if (filename.endsWith(".html")) return "text/html";
	else if (filename.endsWith(".css")) return "text/css";
	else if (filename.endsWith(".js")) return "application/javascript";
	else if (filename.endsWith(".ico")) return "image/x-icon";
	else if (filename.endsWith(".gz")) return "application/x-gzip";
	return "text/plain";
}
void setupWeb() {
	WiFi.softAPConfig(WebInt, WebGetaway, WebSubnet);                 // softAPConfig (local_ip, gateway, subnet)
	WiFi.softAP(web_ssid, web_password, 9, 0, 8);                           // WiFi.softAP(ssid, password, channel, hidden, max_connection)   
	WiFi.hostname("ESP_Bathroom");
	LOG("WIFI < " + String(web_ssid) + " > ... Started");
	LOG("password: " + String(web_password));
	IPAddress IP = WiFi.softAPIP();
	LOG("AccessPoint IP : ");
	LOG(IP);
}
void  setupClient() {
	WiFi.config(Own, TCP_Gateway, TCP_Subnet);
	WiFi.begin(ssid, password);
	int c = TCP_Client.connect(TCP_Server, TCPPort);
	LOG("TCP_Client.localIP() :");
	LOG(TCP_Client.localIP());
	LOG("Client Started " + String(c));
	Print_connection_status();
	Tell_Server_we_are_there();
}

void Print_connection_status() {
	LOG("Connected To      : " + String(WiFi.SSID()));
	LOG("Signal Strenght   : " + String(WiFi.RSSI()) + " dBm");
	LOG("localIP : ");
	LOG(WiFi.localIP());
	LOG("subnetMask() : ");
	LOG(WiFi.subnetMask());
	LOG("Chanel: " + String(WiFi.channel()));
	LOG("macAddress() : ");
	LOG(WiFi.macAddress());
	LOG("WiFi.gatewayIP : ");
	LOG(WiFi.gatewayIP());
}

void Tell_Server_we_are_there() {
	if (TCP_Client.connect(TCP_Server, TCPPort)) {
		//Serial.println("<" + Devicename + "-CONNECTED>");
		sendToServer("<" + Devicename + "-CONNECTED>");
	}
}
void Check_WiFi_and_Connect_or_Reconnect() {
	if (WiFi.status() != WL_CONNECTED) {
		TCP_Client.stop();                                  //Make Sure Everything Is Reset
		WiFi.disconnect();
		setupClient();
		for (int i = 0; i < 10; ++i) {
			if (WiFi.status() != WL_CONNECTED) {
				delay(500);
				//Serial.print(".");
			}
			else {
				Tell_Server_we_are_there();
				break;
			}

		}
		LOG("WiFi isn't connected");
	}
	else LOG("WiFi connected");
}
bool sendToServer(String message) {
	if (!TCP_Client.connected()) {
		if (!TCP_Client.connect(TCP_Server, TCPPort)) {
			LOG("can't connect to server");
			setupClient();
			return false;
		}
	}
	else {
		TCP_Client.setNoDelay(1);
		TCP_Client.println(message);
		LOG("send to server:" + message);
		return true;
	}

}