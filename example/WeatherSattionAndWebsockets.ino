#include <WeatherStationLib.h>
#include <WiFiConnect.h>
#include <WebSocketServer.h>

#define WINDSPEEDPIN 1
#define DIRPIN 15
#define RAINPIN 4

// I need to Call Adafruit library from the this sketch , because when I tried to call the method 
// .begin() the MKR100 hangs. So include the Adafruit library in the WeatherStationLib library 
// but I called that method from setup() and I read the BME280 from my Library. Very weird
extern int bmeError;
extern Adafruit_BME280 bme;

String ssid = "ATT Casa Mia";
String passwd = "mercedes";
int webSocketPort = 51000;
WiFiServer server(webSocketPort);
WebSocketServer webSocketServer;
WiFiConnect wifi(ssid, passwd);
WeatherStation weather(RAINPIN, DIRPIN, WINDSPEEDPIN, 1);

//Print status on serial
void printStatus() {

  if (weather.available()) {

    Serial.print("Speed: ");
    Serial.print(weather.windSpeed());

    Serial.print(" Gust: ");
    Serial.print(weather.windGust());

    Serial.print(" Direction: ");
    Serial.print(weather.windDirection());

    Serial.print(" Rain: ");
    Serial.print(weather.actualRain());

    Serial.print(" Last Rain: ");
    Serial.print(weather.lastRain());

    Serial.print(" Acum. Rain: ");
    Serial.print(weather.accumulatedRain());

    Serial.print(" Last Day Rain: ");
    Serial.print(weather.lastDayRain());

    Serial.print("Temp: ");
    Serial.print(weather.temperature());

    Serial.print(" Hum: ");
    Serial.print(weather.humidity());

    Serial.print(" Press: ");
    Serial.println(weather.pressure());

    Serial.println();
  }

}


// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);
  while (!Serial) {}

  int i;
  // Try 3 times to connect with the wifi
  for (i = 0; i < 3; i++) {
    if (wifi.connect() ) {
      break;
    }
  }

  if (!wifi.connected) {
    Serial.println("Connection Fail!");
      while (true) {}
  }
  else {
    Serial.println("Connected!");
  }

  if (bme.begin())
    bmeError = false;
  else
    bmeError = true;

  // start the server:
  server.begin();

  Serial.println();
  Serial.print("Waiting for Client Connection on IP: "); Serial.print(wifi.ip());
  Serial.print(":"); Serial.println(webSocketPort);
}

// the loop function runs over and over again forever
void loop() {
  String data;
  int i = 0, startTime, sendTime = 10 * 1000;

  //looks if somebody connect
  WiFiClient client = server.available();
    
  if (client.connected() && webSocketServer.handshake(client)) {

    while (client.connected()) {
      data = webSocketServer.getData();

      if (data.length() > 0) {
        Serial.println("Received: " + data);
      }

      //printStatus();
      if (weather.available()) {
        data = "MSGOK," + (String)weather.windSpeed() + "," + (String)weather.windGust() + "," + (String)weather.windDirection() + "," +
          (String)weather.actualRain() + "," + (String)weather.lastDayRain() + "," + (String)weather.accumulatedRain() + "," +
          (String)weather.temperature() + "," + (String)weather.humidity() + "," + (String)weather.pressure();
			
        //we can not use a delay() because conflict with data retrieval
				if (millis() - startTime > sendTime) {
					webSocketServer.sendData(data);
					Serial.print("Sending Data: "); Serial.println(data);
					startTime = millis();
				}
      }
    }
  }

}
