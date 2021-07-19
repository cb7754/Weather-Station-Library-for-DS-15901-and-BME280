#include <WeatherStationLib.h>

#define WINDSPEEDPIN 1
#define DIRPIN 15
#define RAINPIN 4

extern int bmeError;
extern Adafruit_BME280 bme;

WeatherStation weather(RAINPIN, DIRPIN, WINDSPEEDPIN, 1);     //This should configure all pins correctly

int startTime;

void setup() {
	Serial.begin(9600); 
	while (!Serial) {}
	startTime = millis();
	
	Serial.println("Start!");

	//bool statusBME = bme.begin();
	if(bme.begin())
		bmeError = false;
	else
		bmeError = true;
	
}

void loop() {
	
  weather.available();
  delay(500);
    
 //You can not use delay() here because you need to call weather.available() at leat every second 
 if((millis() - startTime) < 10 * 1000) {
	  startTime = millis();
  
    if(weather.available()) {

      Serial.print("Speed: ");
      Serial.print(weather.windSpeed());

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
}
