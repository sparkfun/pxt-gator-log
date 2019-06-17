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


using namespace pxt;

namespace gatorLog {
	MicroBitI2C i2c(I2C_SDA0, I2C_SCL0);
	OpenLog *log; //BME280 handles our Temp, pressure, and humidity
	//#define PXT_STRING_DATA(str) str->data
	#ifndef PXT_STRING_DATA
	#define PXT_STRING_DATA(str) str->data
	#endif
	
	/*
	* Initializes the particle sensor
	*/
	//%
	void begin()
	{
		log->begin();
	}
	
	//%
	void createFile(String value)
	{
		const char * temp = PXT_STRING_DATA(value);

		log->create((char *)temp);
		log->syncFile();
	}
	
	//%
	void openFile(String value)
	{
		const char * temp = PXT_STRING_DATA(value);

		log->append((char *)temp);
		log->syncFile();
	}
	
	//%
	void writeStringData(String value)
	{
		const char * temp = PXT_STRING_DATA(value);

		log->writeString((char *)temp);
		log->syncFile();
	}
	
	//%
	void mkDirectory(String value)
	{
		const char * temp = PXT_STRING_DATA(value);

		log->makeDirectory((char *)temp);
	}
	
	//%
	void chDirectory(String value)
	{
		const char * temp = PXT_STRING_DATA(value);

		log->changeDirectory((char *)temp);
	}
	
	//%
	int32_t sizeOfFile(String value)
	{
		const char * temp = PXT_STRING_DATA(value);

		return log->size((char *)temp);
	}
	
	//%
	bool search(String value)
	{
		const char * temp = PXT_STRING_DATA(value);

		return log->searchDirectory((char *)temp);
	}
	
	//%
	String getNextItem()
	{
		String temp;

		log->getNextDirectoryItem((uint8_t *)temp);
		return temp;
	}
	
	//%
	void removeItem(String value)
	{
		const char * temp = PXT_STRING_DATA(value);

		log->removeFile((char *)temp);
	}
	
	//%
	void removeDir(String value)
	{
		const char * temp = PXT_STRING_DATA(value);

		log->removeDirectory((char *)temp);
	}
}