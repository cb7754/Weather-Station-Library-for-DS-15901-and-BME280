/*
  
  This code is in the public domain.

  modified 14 Jul 2021
  by Carlos Balseiro
 */
 /*
 ** @file		WeatherStation.h
 ** @author		Carlos Balseiro
 ** @copyright	Carlos Balseiro - All rights reserved
 **
 ** This library provides a set of functions for interfacing
 ** with the SparkFun DS-15901 weather station sensor package
 ** These sensors consist of a rain gauge, a wind vane and an
 ** anemometer. Also is added a BME280 chip to measure Temperature,
 ** Humidity and Pressure.

 */

#include "WeatherStationLib.h"

//Outside the Class

//Use Adafruit BME280 library for temp., hum and pressure
Adafruit_BME280 bme;
bool bmeError;


volatile int _speedCounter;
volatile int _rainCounter;
volatile unsigned long microsSpeed;
volatile unsigned long microsRain;

//ISRs
int rainCounter=1;
static void countRain()
{

	if ((long)(micros() - microsRain) >= DEBOUNCE * 1000)
	{
		_rainCounter++;
		microsRain = micros();
		//Serial.println((String) rainCounter++ + " " + (String)(millis() / 1000));
	}
	
}

//ISR for anemometer.
static void countAnemometer()
{
	if ((long)(micros() - microsSpeed) >= DEBOUNCE * 1000)
	{
		_speedCounter++;
		//speedCounter++;
		microsSpeed = micros();
		//Serial.println((String) speedCounter + " " + (String) (millis() / 1000) );
	}
}

//Return seconds fron the begining of the program
int WeatherStation::_secs()
{
	return millis() / 1000 - _initTime;	
}

int WeatherStation::_calibrate(int speed){
	return (int) ( speed * SLOPE );
}
void WeatherStation::_getBM280Sensors()
{
	if (bmeError) 
	{
		_temperature = -1;
		_humidity = -1;
		_pressure = -1;
	}
	else
	{
		_temperature = bme.readTemperature();
		_humidity = (int) bme.readHumidity();
		_pressure = bme.readPressure() / 100;
	}
	
}
//Initialize Cardinal points array
void WeatherStation::_initDirArray() {
	_dirRes[0] = { "ESE", 0.7 };
	_dirRes[1] = { "ENE", 0.9 }; 
	_dirRes[2] = { "E", 1.0 };
	_dirRes[3] = { "SSE", 1.4 };
	_dirRes[4] = { "SE", 2.2 };
	_dirRes[5] = { "SSW", 3.1 };
	_dirRes[6] = { "S", 3.9 };
	_dirRes[7] = { "NNE", 6.6 };
	_dirRes[8] = { "NE", 8.2 };
	_dirRes[9] = { "WSW", 14.1 };
	_dirRes[10] = { "SW", 16.0 };
	_dirRes[11] = { "NNW", 21.9 };
	_dirRes[12] = { "N", 33.0 };
	_dirRes[13] = { "WNW", 42.1 };
	_dirRes[14] = { "NW", 64.9 };
	_dirRes[15] = { "W", 120.0 };
}

//Look up the dirRes array for resistance and return a string direction
String WeatherStation::_searchDirection(double key) {
	int n = 16;
	int low;
	int high;
	char keychar[100];
	char* ptr;
	double midVal;
	int mid, i;

	while (true) {
		//printf("\nInput number: ");
		////scanf("%s", keychar);
		//scanf_s("%s", keychar, 10);
		//key = strtod(keychar, &ptr);

		low = 0;
		high = n - 1;

		while (low <= high) {
			mid = (low + high) / 2;
			midVal = _dirRes[mid].res;

			if (midVal < key)
				low = mid + 1;
			else if (midVal > key)
				high = mid - 1;
			else {
				//printf("%f is present at index %d value is %f and direction is %s.\n", key, mid, _dirRes[mid].res, _dirRes[mid].dir); // key found
				return _dirRes[mid].dir;
			}
		}

		midVal = (_dirRes[low].res - _dirRes[low - 1].res) / 2 + _dirRes[low - 1].res;
		if (midVal < key)
			mid = low;
		else if (low > 0)
			mid = low - 1;
		else
			mid = low;
		if (mid > n - 1)
			mid = n - 1;

		//printf("%f is approx. at index %d, value:%f and direction is %s.\n", key, mid, _dirRes[mid].res, _dirRes[mid].dir);  // key not found.
		return _dirRes[mid].dir;
	}
}

//Calculate the rain fall 
int WeatherStation::_rainFall() 
{
	//measurement intervalin seconds
	int counts = _rainCounter;
	return (double)counts * KRAIN / 1.0; //one pin change x bucket
}

//Calcule the actualRain(rain that fall until it sops for a least 30 minutes), the lastRain and the accumulated rain.
void WeatherStation::_getRain()
{
	
	int now, rain;
	now = _secs();
	rain = _rainFall();
  
	if (_actualRain < rain || rain == 0)
		_actualRain = rain;
	else
	{
		if (!_rainStop)
		{
			_rainStopTime = now;
			_rainStop = true;
		}
		if ((now - _rainStopTime) > 10*60)  //if rains stop 10 min then set _lastRain and accumulate in _sumRain
		{
			_rainCounter = 0;
			_rainStop = false;
			_lastRain = _actualRain;
			_rainStopTime = 0;
			_sumRain += _lastRain;
		}
	}
	if ((_secs() - _lastRainPeriod) > _accumulatedRainPeriod * 24 * 3600) //_accumulatedRainPeriod in days
	{
		_lastRainPeriod = _secs();
		_lastDayRain = _accumulatedRainPeriod;
		_accumulatedRainPeriod = 0;
	}
		
}

//Calculate Wind Direction and return a string with the cardinal point.
void WeatherStation::_getDir()
{
	int i, j, sumADC;
	double V, R;
	double sumr = 0;
	String dir;
  
	sumADC = 0;
	for (i = 0; i < 10; i++)
		sumADC += analogRead(_dirPin);
	
	V = VREF * (double)sumADC / 10.0 / 1023.0;
	R =  V * 9740.0 / (VREF - V) / 1000.0;
	_dir =   _searchDirection(R);
	
}

//returns the wind speed during 2 seconds
int WeatherStation::_getSpeed2Sec()
{
	int startTime2, counts;
	int speed; 

	startTime2 = millis();
	_speedCounter = 0;
	while ((millis() - startTime2) < 2 * 1000) {}
	counts = _speedCounter;
		
	_countSum += counts;
	speed = counts / 2.0 * KSPEED;
	_instantSpeed[_gustIdx++] = speed;
}

//To calculate gust from an array of speeds taken every 2 seconds
double WeatherStation::_maxSpeed()
{
	int i;
	double maxi = _instantSpeed[0];
  
	for (i = 1; i < _gustIdx - 1; i++)
		if (maxi < _instantSpeed[i])
			maxi = _instantSpeed[i];

	return maxi;
}

//Constructor
WeatherStation::WeatherStation(int rainPin, int dirPin, int speedPin, int accumulatedRainPeriod)
{
	//Initialization routine
	_speedCounter = 0;
	_rainCounter = 0;
	_rainPin = rainPin;
	_dirPin = dirPin;
	_speedPin = speedPin;
	_accumulatedRainPeriod = accumulatedRainPeriod;  //in days

	pinMode(_rainPin, INPUT);
	digitalWrite(_rainPin, HIGH);
	pinMode(_speedPin, INPUT);
	digitalWrite(_speedPin, HIGH);

	pinMode(_dirPin, INPUT);
	
	attachInterrupt(digitalPinToInterrupt(_rainPin), countRain, FALLING);     //ws1.countRain is the ISR for the rain gauge.
	attachInterrupt(digitalPinToInterrupt(_speedPin), countAnemometer, FALLING);     //ws1.countAnemometer is the ISR for the anemometer.
	
	_initDirArray();
	_startTime = millis();
	_initTime = millis();
	_rainStop = false;
	_rainStopTime = 0;
	_actualRain = 0;
	_lastRainPeriod = _secs();
	_lastDayRain = 0;
}

//Updates all sensor variables. You must run it at least every second
bool WeatherStation::available()
{
	double speed;
	
		_getSpeed2Sec();
	_getDir();
	_getRain();
	_getBM280Sensors();

	if (millis() - _startTime > INTERVAL * 1000)
	{
		_startTime = _startTime + INTERVAL * 1000;
		_speed = _countSum * KSPEED / INTERVAL;
		//_speedAdj = (int)(_speed * SLOPE);
		_countSum=0;
		_gust = _maxSpeed();
		//_gust = (int) (_gust * SLOPE);
		_gustIdx=0;
		_countSum = 0;
		return true;
	}
		
}

//Returns the rain that fall until it sops for a least 30 minutes
int WeatherStation::actualRain()
{
	return _actualRain;
}

//Return the last rain before the last rain stop
int WeatherStation::lastRain()
{
	return _lastRain;
}

//Return the accumulated rain in in the period: _accumulatedRainPeriod
int WeatherStation::accumulatedRain() 
{
	return _sumRain;
}

int WeatherStation::lastDayRain() 
{
	return _lastDayRain;
}

//Returns the direction of the wind in degrees.
String WeatherStation::windDirection()
{
	return _dir;
}

//Returns the wind speed.
int WeatherStation::windSpeed()
{
	return _speed;
}

//Returns the gust speed. 
int WeatherStation::windGust()
{
	return _gust;
}

float WeatherStation::temperature()
{
	return _temperature;
}

int WeatherStation::humidity()
{
	return _humidity;
}

float WeatherStation::pressure()
{
	return _pressure;
}



