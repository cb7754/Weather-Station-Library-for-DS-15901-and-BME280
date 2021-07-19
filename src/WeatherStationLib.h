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

#include "Arduino.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>


#define KSPEED 2.49 // KSPEED based in 4 Close per turn and 7cm radius
#define KRAIN 0.2794  // 0.2794mm of rain per _rainCounter
#define DEBOUNCE 15 //microseconds
#define INTERVAL 10 // seconds
#define VREF 3.27   //volts
#define SLOPE 0.73	// to use in WindSpeed calibration


class WeatherStation
{
public:
  WeatherStation(int rainPin, int windDirPin, int windSpdPin, int accumulatedRainPeriod);

  bool available();
  int actualRain();
  int lastRain();
  int accumulatedRain();
  int lastDayRain();
  String windDirection();
  int windSpeed();
  int windGust();
  float temperature();
  int humidity();
  float pressure();


private:
  int _rainPin, _dirPin, _speedPin, _rain, _startTime, _speedCount, _countSum, _start, _gustIdx, _rainStopTime, _initTime;
  int _speed, _gust, _instantSpeed[100], _accumulatedRainPeriod, _lastRainPeriod, _lastDayRain, _humidity, _speedAdj;
  double _actualRain, _lastRain, _sumRain;
  float _temperature, _pressure;
  String _dir;
  bool _rainStop;

  int _secs();
  void _getRain();
  void _getDir();
  int _getSpeed2Sec();
  double _maxSpeed();
  void _initDirArray();
  String _searchDirection(double key);
  int _rainFall();
  void _getBM280Sensors();
  int _calibrate(int speed);

  typedef struct direction {
    String dir;
    double res;
  } Direction;

  Direction _dirRes[16];
};
