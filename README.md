# Weather Station Library to use with DS-15901 and BME280 for Arduino

This library is to use with the SparkFun DS-15901 weather station (wind direction, wind speed and rain fall) and the BME280 sensors (Temperature, humidity and pressure).
To use you must downloaded, install and add in your main Sketch the following line at the beginning:
`#include "WeatherStationLib.h`


then add:

`extern int bmeError;`

`extern Adafruit_BME280 bme;`

`WeatherStation weather(rain_Pin, wind_direction_Pin, wind_speed_Pin, accumulated_rain_period)`

where,
  wind_direction_Pin        = The digital pin you to measure wind direction
  wind_speed_Pin            = The digital pin you to measure wind speed            
  rain_Pin                  = The analog pin you to measure rain fell
  accumulated_rain_period   = The period of time (in days) you going to use to measure accumulated rain.
  
You must call in your loop() the weather.available() method at least once per second.
The following method can be used:


  - int actualRain()             = The actual rain fell in the last call to available() method in mm. 
  
  - int lastRain()               = the last rain from the rain stop for at least 10 minutes in mm.
  
  - int accumulatedRain()        = The accumulated rain fell in the accumulated_rain_period in mm.
  - int lastDayRain()            = The rain of the last day in mm. 
  - String windDirection()       = Wind direction in a string form, i.e.: "NNE".
  - int windSpeed()              = Wind speed in Km/h.
  - int windGust()               = Wind gust in Km/h.
  - float temperature()          = Outside temperature in Celsius.
  
  I tried to compile the library with the Adafruit BME280 library and use their functionality inside my library, but when it did it the Arduino hangs. Specifically, it hangs when I 
  call the method .begin(). So, I ended doing this unorthodox way of include the <Adafruit_BME280.h> in my library and the call the begin() method from the main sketch. This is the reason i Had to declare those variables as extern.
  
