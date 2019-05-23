/**
* Andy England @ SparkFun Electronics
* September 6, 2018
* https://github.com/sparkfun/pxt-light-bit
*
* Development environment specifics:
* Written in Microsoft PXT
* Tested with a SparkFun temt6000 sensor and micro:bit
*
* This code is released under the [MIT License](http://opensource.org/licenses/MIT).
* Please review the LICENSE.md file included with this example. If you have any questions
* or concerns with licensing, please contact techsupport@sparkfun.com.
* Distributed as-is; no warranty is given.
*/


#include "pxt.h"
#include <cstdint>
#include <math.h>
#include "SparkFun_Qwiic_OpenLog_Arduino_Library.h"
#include "ManagedString.h"

using namespace pxt;

namespace gatorLog {
	MicroBitI2C i2c(I2C_SDA0, I2C_SCL0);
	OpenLog *log; //BME280 handles our Temp, pressure, and humidity
	
	/*
	* Initializes the particle sensor
	*/
	//%
	void begin()
	{
		log->begin();
	}
	
	//%
	void createFile(char buffer)
	{
		log->create(&buffer);
	}
}