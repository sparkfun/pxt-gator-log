/*
  This is a library written for the Qwiic OpenLog
  SparkFun sells these at its website: www.sparkfun.com
  Do you like this library? Help support SparkFun. Buy a board!
  https://www.sparkfun.com/products/14641
  Written by Nathan Seidle @ SparkFun Electronics, February 2nd, 2018
  Qwiic OpenLog makes it very easy to record data over I2C to a microSD.
  This library handles the initialization of the Qwiic OpenLog and the calculations
  to get the temperatures.
  https://github.com/sparkfun/SparkFun_Qwiic_OpenLog_Arduino_Library
  Development environment specifics:
  Arduino IDE 1.8.3
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "SparkFun_Qwiic_OpenLog_Arduino_Library.h"
#include "mbed.h"
#include "MicroBit.h"
#include "ManagedString.h"
#include <vector>

static MicroBitI2C i2c(I2C_SDA0, I2C_SCL0);

#define RV3028_ADDR						0b1010010

//The upper part of the part number is always 0x18
#define RV3028_PART_NUMBER_UPPER		0x18

//Possible CONFKEY Values
#define RV3028_CONF_RST					0x3C //value written to Configuration Key for reset
#define RV3028_CONF_OSC					0xA1 //value written to Configuration Key for oscillator control register write enable
#define RV3028_CONF_WRT					0x9D //value written to Configuration Key to enable write of trickle charge, BREF, CAPRC, IO Batmode, and Output Control Registers

//Bits in Control1 Register
#define CTRL1_STOP	7
#define CTRL1_12_24	6
#define CTRL1_PSWB	5
#define CTRL1_ARST						1 << 2 //Enables reset of interrupt flags in status register 

//Bits in Hours register
#define HOURS_AM_PM						5

//Interrupt Enable Bits
#define INTERRUPT_BLIE	4
#define INTERRUPT_TIE	3
#define INTERRUPT_AIE	2
#define INTERRUPT_EIE	1


//PSW Pin Function Selection Bits
#define PSWS_OFFSET     2
#define PSWS_INV_IRQ    0b000
#define PSWS_SQW        0b001
#define PSWS_INV_AIRQ   0b011
#define PSWS_TIRQ       0b100
#define PSWS_INV_TIRQ   0b101
#define PSWS_SLEEP      0b110
#define PSWS_STATIC     0b111

//Countdown Timer Control
#define COUNTDOWN_SECONDS		0b10
#define COUNTDOWN_MINUTES		0b11
#define CTDWN_TMR_TE_OFFSET		7
#define CTDWN_TMR_TM_OFFSET		6
#define CTDWN_TMR_TRPT_OFFSET	5


//Status Bits
#define STATUS_CB	7
#define STATUS_BAT 6
#define STATUS_WDF 5
#define STATUS_BLF 4
#define STATUS_TF 3
#define STATUS_AF 2
#define STATUS_EVF 1

//Reference Voltage
#define TWO_FIVE						0x70
#define TWO_ONE							0xB0
#define ONE_EIGHT						0xD0
#define ONE_FOUR						0xF0

//Register names:
#define RV3028_SECONDS					0x00
#define RV3028_MINUTES					0x01
#define RV3028_HOURS					0x02
#define RV3028_WEEKDAYS					0x03
#define RV3028_DATE         			0x04
#define RV3028_MONTHS        			0x05
#define RV3028_YEARS        			0x06
#define RV3028_MINUTES_ALM     			0x07
#define RV3028_HOURS_ALM       			0x08
#define RV3028_WEEKDAYS_ALM    			0x09
#define RV3028_DATE_ALM					0x09
#define RV3028_TIMER_0					0x0A
#define RV3028_TIMER_1					0x0B
#define RV3028_TIMER_STATUS_0			0x0C
#define RV3028_TIMER_STATUS_1			0x0D
#define RV3028_STATUS					0x0E
#define RV3028_CTRL1					0x0F
#define RV3028_CTRL2					0x10
#define RV3028_GP_BITS					0x11
#define RV3028_INT_MASK					0x12
#define RV3028_EVENT_CTRL				0x13
#define RV3028_COUNT_TS					0x14
#define RV3028_SECONDS_TS				0x15
#define RV3028_MINUTES_TS				0x16
#define RV3028_HOURS_TS					0x17
#define RV3028_DATE_TS					0x18
#define RV3028_MONTH_TS					0x19
#define RV3028_YEAR_TS					0x1A
#define RV3028_UNIX_TIME_0				0x1B
#define RV3028_UNIX_TIME_1				0x1C
#define RV3028_UNIX_TIME_2				0x1D
#define RV3028_UNIX_TIME_3				0x1E
#define RV3028_USER_RAM_1				0x1F
#define RV3028_USER_RAM_2				0x20
#define RV3028_PASSWORD_0				0x21
#define RV3028_PASSWORD_1				0x22
#define RV3028_PASSWORD_2				0x23
#define RV3028_PASSWORD_3				0x24
#define RV3028_EEPROM_ADDR				0x25
#define RV3028_EEPROM_DATA				0x26
#define RV3028_EEPROM_CMD				0x27
#define RV3028_ID						0x28


char SLAVE_ADDRESS =   0x54;

static const char LOG_ID = 			0x00;
static const char LOG_STATUS = 		0x01;
static const char LOG_FWMINOR =		0x02;
static const char LOG_FWMAJOR =		0x03;
static const char LOG_I2CADDRESS =	0x1E;
static const char LOG_INIT =		0x05;
static const char LOG_CREATE_FILE =	0x06;
static const char LOG_MKDIR =		0x07;
static const char LOG_CD =			0x08;
static const char LOG_READ_FILE =	0x09;
static const char LOG_START_POS =	0x0A;
static const char LOG_OPEN_FILE =	0x0B;
static const char LOG_WRITE_FILE =	0x0C;
static const char LOG_FILE_SIZE =	0x0D;
static const char LOG_LIST =		0x0E;
static const char LOG_RM =			0x0F;
static const char LOG_RMRF =		0x10;
static const char LOG_SYNC_FILE =	0x11;
static const char cmdready = 		0x12;
static const char continueRead = 	0x13;

static const char I2C_BUFFER_LENGTH = 32;

// Parse the __DATE__ predefined macro to generate date defaults:
// __Date__ Format: MMM DD YYYY (First D may be a space if <10)
// <MONTH>
#define BUILD_MONTH_JAN ((__DATE__[0] == 'J') && (__DATE__[1] == 'a')) ? 1 : 0
#define BUILD_MONTH_FEB (__DATE__[0] == 'F') ? 2 : 0
#define BUILD_MONTH_MAR ((__DATE__[0] == 'M') && (__DATE__[1] == 'a') && (__DATE__[2] == 'r')) ? 3 : 0
#define BUILD_MONTH_APR ((__DATE__[0] == 'A') && (__DATE__[1] == 'p')) ? 4 : 0
#define BUILD_MONTH_MAY ((__DATE__[0] == 'M') && (__DATE__[1] == 'a') && (__DATE__[2] == 'y')) ? 5 : 0
#define BUILD_MONTH_JUN ((__DATE__[0] == 'J') && (__DATE__[1] == 'u') && (__DATE__[2] == 'n')) ? 6 : 0
#define BUILD_MONTH_JUL ((__DATE__[0] == 'J') && (__DATE__[1] == 'u') && (__DATE__[2] == 'l')) ? 7 : 0
#define BUILD_MONTH_AUG ((__DATE__[0] == 'A') && (__DATE__[1] == 'u')) ? 8 : 0
#define BUILD_MONTH_SEP (__DATE__[0] == 'S') ? 9 : 0
#define BUILD_MONTH_OCT (__DATE__[0] == 'O') ? 10 : 0
#define BUILD_MONTH_NOV (__DATE__[0] == 'N') ? 11 : 0
#define BUILD_MONTH_DEC (__DATE__[0] == 'D') ? 12 : 0
#define BUILD_MONTH BUILD_MONTH_JAN | BUILD_MONTH_FEB | BUILD_MONTH_MAR | \
BUILD_MONTH_APR | BUILD_MONTH_MAY | BUILD_MONTH_JUN | \
BUILD_MONTH_JUL | BUILD_MONTH_AUG | BUILD_MONTH_SEP | \
BUILD_MONTH_OCT | BUILD_MONTH_NOV | BUILD_MONTH_DEC
// <DATE>
#define BUILD_DATE_0 ((__DATE__[4] == ' ') ? 0 : (__DATE__[4] - 0x30))
#define BUILD_DATE_1 (__DATE__[5] - 0x30)
#define BUILD_DATE ((BUILD_DATE_0 * 10) + BUILD_DATE_1)
// <YEAR>
#define BUILD_YEAR (((__DATE__[7] - 0x30) * 1000) + ((__DATE__[8] - 0x30) * 100) + \
((__DATE__[9] - 0x30) * 10)  + ((__DATE__[10] - 0x30) * 1))

// Parse the __TIME__ predefined macro to generate time defaults:
// __TIME__ Format: HH:MM:SS (First number of each is padded by 0 if <10)
// <HOUR>
#define BUILD_HOUR_0 ((__TIME__[0] == ' ') ? 0 : (__TIME__[0] - 0x30))
#define BUILD_HOUR_1 (__TIME__[1] - 0x30)
#define BUILD_HOUR ((BUILD_HOUR_0 * 10) + BUILD_HOUR_1)
// <MINUTE>
#define BUILD_MINUTE_0 ((__TIME__[3] == ' ') ? 0 : (__TIME__[3] - 0x30))
#define BUILD_MINUTE_1 (__TIME__[4] - 0x30)
#define BUILD_MINUTE ((BUILD_MINUTE_0 * 10) + BUILD_MINUTE_1)
// <SECOND>
#define BUILD_SECOND_0 ((__TIME__[6] == ' ') ? 0 : (__TIME__[6] - 0x30))
#define BUILD_SECOND_1 (__TIME__[7] - 0x30)
#define BUILD_SECOND ((BUILD_SECOND_0 * 10) + BUILD_SECOND_1)

using namespace std;

//Attempt communication with the device
//Return true if we got a 'Polo' back from Marco
void OpenLog::begin()
{
  //We require caller to begin their I2C port, with the speed of their choice
  //external to the library
  //_i2cPort->begin();

  //Check communication with device
  getStatus();
  getVersion();
}

//Get the version number from OpenLog
uint16_t OpenLog::getVersion()
{
  uint8_t versionMajor = readRegister(SLAVE_ADDRESS, LOG_FWMAJOR); 
  uint8_t versionMinor = readRegister(SLAVE_ADDRESS, LOG_FWMINOR); 

  return (versionMajor << 8) | versionMinor;
}

//Get the status uint8_t from OpenLog
//This function assumes we are not in the middle of a read, file size, or other function
//where OpenLog has uint8_ts qued up
//  Bit 0: SD/Init Good
//  Bit 1: Last Command Succeeded
//  Bit 2: Last Command Known
//  Bit 3: File Currently Open
//  Bit 4: In Root Directory
//  Bit 5: 0 - Future Use
//  Bit 6: 0 - Future Use
//  Bit 7: 0 - Future Use
uint8_t OpenLog::getStatus()
{
  //sendCommand(LOG_STATUS, "");

  return readRegister(SLAVE_ADDRESS, LOG_STATUS);
}

//Change the I2C address of the OpenLog
//This will be recorded to OpenLog's EEPROM and config.txt file.
bool OpenLog::setI2CAddress(char addr)
{
  sendCommand(LOG_I2CADDRESS, &addr);

  //Upon completion any new communication must be with this new I2C address  

  SLAVE_ADDRESS = addr; //Change the address internally

  return(true);
}

//Append to a given file. If it doesn't exist it will be created
void OpenLog::append(char *fileName)
{
  sendCommand(LOG_OPEN_FILE, fileName);
  //Upon completion any new characters sent to OpenLog will be recorded to this file
}

//Create a given file in the current directory
void OpenLog::create(char *fileName)
{
  sendCommand(LOG_CREATE_FILE, fileName);//Correctly passing things into sendCommand, is fileName properly passed into here though?
  //Upon completion a new file is created but OpenLog is still recording to original file
}

//Given a directory name, create it in whatever directory we are currently in
void OpenLog::makeDirectory(char *directoryName)
{
  sendCommand(LOG_MKDIR, directoryName);
  //Upon completion Qwiic OpenLog will respond with its status
  //Qwiic OpenLog will continue logging whatever it next receives to the current open log
}

//Given a directory name, change to that directory
void OpenLog::changeDirectory(char *directoryName)
{
  sendCommand(LOG_CD, directoryName);
  //Upon completion Qwiic OpenLog will respond with its status
  //Qwiic OpenLog will continue logging whatever it next receives to the current open log
}

//Return the size of a given file. Returns a 4 uint8_t signed long
int32_t OpenLog::size(char *fileName)
{
  sendCommand(LOG_FILE_SIZE, fileName);
  //Upon completion Qwiic OpenLog will have 4 uint8_ts ready to be read

  //_i2cPort->requestFrom(SLAVE_ADDRESS, (uint8_t)4);

  uint8_t data[4];
  readRegisterRegion(SLAVE_ADDRESS, data, LOG_FILE_SIZE, 4);
  int32_t fileSize = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];

  return (fileSize);
}

//Read the contents of a file, up to the size of the buffer, into a given array, from a given spot
void OpenLog::read(uint8_t* userBuffer, uint16_t bufferSize, char fileName[])
{
  uint16_t spotInBuffer = 0;
  uint16_t leftToRead = bufferSize; //Read up to the size of our buffer. We may go past EOF.
  sendCommand(LOG_READ_FILE, fileName);
  //Upon completion Qwiic OpenLog will respond with the file contents. Master can request up to 32 uint8_ts at a time.
  //Qwiic OpenLog will respond until it reaches the end of file then it will report zeros.

  while (leftToRead > 0)
  {
    uint8_t toGet = I2C_BUFFER_LENGTH; //Request up to a 32 uint8_t block
    if (leftToRead < toGet) toGet = leftToRead; //Go smaller if that's all we have left

    /*_i2cPort->requestFrom(SLAVE_ADDRESS, toGet);
    while (_i2cPort->available())
      userBuffer[spotInBuffer++] = _i2cPort->read();
	*/
	readRegisterRegion(SLAVE_ADDRESS, &userBuffer[spotInBuffer], LOG_READ_FILE, toGet);
    leftToRead -= toGet;
	spotInBuffer += toGet;
  }
}

//Read the contents of a directory. Wildcards allowed
//Returns true if OpenLog ack'd. Use getNextDirectoryItem() to get the first item.
bool OpenLog::searchDirectory(char *options)
{
  sendCommand(LOG_LIST, options);
  _searchStarted = true;
  return (true);
    //Upon completion Qwiic OpenLog will have a file name or directory name ready to respond with, terminated with a \0
    //It will continue to respond with a file name or directory until it responds with all 0xFFs (end of list)
}

//Returns the name of the next file or directory folder in the current directory
//Returns "" if it is the end of the list
void OpenLog::getNextDirectoryItem(uint8_t* userBuffer)
{

  //std::vector<char> itemName;
  //_i2cPort->requestFrom(SLAVE_ADDRESS, (uint8_t)I2C_BUFFER_LENGTH);
  char tempData;
  uint8_t position = 0;
  //uint8_t charsReceived = 0;
  while (tempData != '\0')
  {
	i2c.read(SLAVE_ADDRESS, &tempData, 1, true);
	userBuffer[position++] = tempData;
  }
  /*while (_i2cPort->available())
  {
    uint8_t incoming = _i2cPort->read();
    if (incoming == '\0')
      return (itemName); //This is the end of the file name. We don't need to read any more of the 32 uint8_ts
    else if (charsReceived == 0 && incoming == 0xFF)
    {
      _searchStarted = false;
      return (""); //End of the directory listing
    }
    else
      itemName += (char)incoming; //Add this uint8_t to the file name
    charsReceived++;
  }*/
  /*char itemNameArray[itemName.size()];
  for (uint8_t copy = 0; copy < itemName.size(); copy++)
  {
	itemNameArray[copy] = itemName[copy];
  }
  char *itemNameToReturn(itemNameArray);
  //We shouldn't get this far but if we do
  return(itemNameToReturn);*/
}

//Remove a file, wildcards supported
//OpenLog will respond with the number of items removed
uint32_t OpenLog::removeFile(char *thingToDelete)
{
	return(remove(thingToDelete, false));
}

//Remove a directory, wildcards supported
//OpenLog will respond with 1 when removing a directory
uint32_t OpenLog::removeDirectory(char *thingToDelete)
{
	return(remove(thingToDelete, true)); //Delete all files in the directory as well
}

//Remove a file or directory (including everything in that directory)
//OpenLog will respond with the number of items removed
//Returns 1 if only a directory is removed (even if directory had files in it)
uint32_t OpenLog::remove(char *thingToDelete, bool removeEverything)
{
  char data[4];
  if(removeEverything == true)
  {
	sendCommand(LOG_RMRF, thingToDelete); //-rf causes any directory to remove contents as well
    i2c.read(SLAVE_ADDRESS, data, 4);
  }
  else
  {
	sendCommand(LOG_RM, thingToDelete); //Just delete a thing
    i2c.read(SLAVE_ADDRESS, data, 4);
  }
  //Upon completion Qwiic OpenLog will have 4 uint8_ts ready to read, representing the number of files beleted

  int32_t filesDeleted = 0;
  
  filesDeleted = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];

  return (filesDeleted); //Return the number of files removed

  //Qwiic OpenLog will continue logging whatever it next receives to the current open log
}



//Configure RTC to output 1-12 hours
//Converts any current hour setting to 12 hour
void OpenLog::set12Hour()
{
	//Do we need to change anything?
	if(is12Hour() == false)
	{		
		uint8_t hour = BCDtoDEC(readRegister(RV3028_ADDR, RV3028_HOURS)); //Get the current hour in the RTC

		//Set the 12/24 hour bit
		uint8_t setting = readRegister(RV3028_ADDR, RV3028_CTRL1);
		setting |= (1<<CTRL1_12_24);
		writeRegister(RV3028_ADDR, RV3028_CTRL1, setting);

		//Take the current hours and convert to 12, complete with AM/PM bit
		bool pm = false;

		if(hour == 0)
			hour += 12;
		else if(hour == 12)
			pm = true;
		else if(hour > 12)
		{
			hour -= 12;
			pm = true;
		}
		
		hour = DECtoBCD(hour); //Convert to BCD

		if(pm == true) hour |= (1<<HOURS_AM_PM); //Set AM/PM bit if needed

		writeRegister(RV3028_ADDR, RV3028_HOURS, hour); //Record this to hours register
	}
}

//Configure RTC to output 0-23 hours
//Converts any current hour setting to 24 hour
void OpenLog::set24Hour()
{
	//Do we need to change anything?
	if(is12Hour() == true)
	{		
		//Not sure what changing the CTRL1 register will do to hour register so let's get a copy
		uint8_t hour = readRegister(RV3028_ADDR, RV3028_HOURS); //Get the current 12 hour formatted time in BCD
		bool pm = false;
		if(hour & (1<<HOURS_AM_PM)) //Is the AM/PM bit set?
		{
			pm = true;
			hour &= ~(1<<HOURS_AM_PM); //Clear the bit
		}
		
		//Change to 24 hour mode
		uint8_t setting = readRegister(RV3028_ADDR, RV3028_CTRL1);
		setting &= ~(1<<CTRL1_12_24); //Clear the 12/24 hr bit
		writeRegister(RV3028_ADDR, RV3028_CTRL1, setting);

		//Given a BCD hour in the 1-12 range, make it 24
		hour = BCDtoDEC(hour); //Convert core of register to DEC
		
		if(pm == true) hour += 12; //2PM becomes 14
		if(hour == 12) hour = 0; //12AM stays 12, but should really be 0
		if(hour == 24) hour = 12; //12PM becomes 24, but should really be 12

		hour = DECtoBCD(hour); //Convert to BCD

		writeRegister(RV3028_ADDR, RV3028_HOURS, hour); //Record this to hours register
	}
}

//Returns true if RTC has been configured for 12 hour mode
bool OpenLog::is12Hour()
{
	uint8_t controlRegister = readRegister(RV3028_ADDR, RV3028_CTRL1);
	return(controlRegister & (1<<CTRL1_12_24));
}

//Returns true if RTC has PM bit set and 12Hour bit set
bool OpenLog::isPM()
{
	uint8_t hourRegister = readRegister(RV3028_ADDR, RV3028_HOURS);
	if(is12Hour() && (hourRegister & (1<<HOURS_AM_PM)))
		return(true);
	return(false);
}

//Returns the status byte. This likely clears the interrupts as well.
//See .begin() for ARST bit setting
uint8_t OpenLog::status(void)
{
	return(readRegister(RV3028_ADDR, RV3028_STATUS));
}

//Returns a pointer to array of chars that are the date in mm/dd/yyyy format because we're weird
char* OpenLog::stringDateUSA()
{
	static char date[11]; //Max of mm/dd/yyyy with \0 terminator
	sprintf(date, "%02d/%02d/20%02d", BCDtoDEC(_time[TIME_MONTH]), BCDtoDEC(_time[TIME_DATE]), BCDtoDEC(_time[TIME_YEAR]));
	return(date);
}

//Returns a pointer to array of chars that are the date in dd/mm/yyyy format
char*  OpenLog::stringDate()
{
	static char date[11]; //Max of dd/mm/yyyy with \0 terminator
	sprintf(date, "%02d/%02d/20%02d", BCDtoDEC(_time[TIME_DATE]), BCDtoDEC(_time[TIME_MONTH]), BCDtoDEC(_time[TIME_YEAR]));
	return(date);
}

//Returns a pointer to array of chars that represents the time in hh:mm:ss format
//Adds AM/PM if in 12 hour mode
char* OpenLog::stringTime()
{
	static char time[11]; //Max of hh:mm:ssXM with \0 terminator

	if(is12Hour() == true)
	{
		char half = 'A';
		if(isPM()) half = 'P';
		
		sprintf(time, "%02d:%02d:%02d%cM", BCDtoDEC(_time[TIME_HOURS]), BCDtoDEC(_time[TIME_MINUTES]), BCDtoDEC(_time[TIME_SECONDS]), half);
	}
	else
	sprintf(time, "%02d:%02d:%02d", BCDtoDEC(_time[TIME_HOURS]), BCDtoDEC(_time[TIME_MINUTES]), BCDtoDEC(_time[TIME_SECONDS]));
	
	return(time);
}

char* OpenLog::stringTimeStamp()
{
	static char timeStamp[21]; //Max of yyyy-mm-ddThh:mm:ss with \0 terminator

	sprintf(timeStamp, "20%02d-%02d-%02dT%02d:%02d:%02d", BCDtoDEC(_time[TIME_YEAR]), BCDtoDEC(_time[TIME_MONTH]), BCDtoDEC(_time[TIME_DATE]), BCDtoDEC(_time[TIME_HOURS]), BCDtoDEC(_time[TIME_MINUTES]), BCDtoDEC(_time[TIME_SECONDS]));
	
	return(timeStamp);
}

void OpenLog::setTime(uint8_t sec, uint8_t min, uint8_t hour, uint8_t date, uint8_t month, uint8_t year, uint8_t day)
{
	_time[TIME_SECONDS] = DECtoBCD(sec);
	_time[TIME_MINUTES] = DECtoBCD(min);
	_time[TIME_HOURS] = DECtoBCD(hour);
	_time[TIME_DATE] = DECtoBCD(date);
	_time[TIME_DAY] = DECtoBCD(day);
	_time[TIME_MONTH] = DECtoBCD(month);
	_time[TIME_YEAR] = DECtoBCD(year);
		
	return setTime(_time, TIME_ARRAY_LENGTH);
}

// setTime -- Set time and date/day registers of RV3028 (using data array)
void OpenLog::setTime(uint8_t * time, uint8_t len)
{	
	writeMultipleRegisters(RV3028_ADDR, RV3028_SECONDS, time, len);
}

void OpenLog::setSeconds(uint8_t value)
{
	_time[TIME_SECONDS] = DECtoBCD(value);
	setTime(_time, TIME_ARRAY_LENGTH);
}

void OpenLog::setMinutes(uint8_t value)
{
	_time[TIME_MINUTES] = DECtoBCD(value);
	setTime(_time, TIME_ARRAY_LENGTH);
}

void OpenLog::setHours(uint8_t value)
{
	_time[TIME_HOURS] = DECtoBCD(value);
	setTime(_time, TIME_ARRAY_LENGTH);
}

void OpenLog::setDate(uint8_t value)
{
	_time[TIME_DATE] = DECtoBCD(value);
	setTime(_time, TIME_ARRAY_LENGTH);
}

void OpenLog::setMonth(uint8_t value)
{
	_time[TIME_MONTH] = DECtoBCD(value);
	setTime(_time, TIME_ARRAY_LENGTH);
}

void OpenLog::setYear(uint8_t value)
{
	_time[TIME_YEAR] = DECtoBCD(value);
	setTime(_time, TIME_ARRAY_LENGTH);
}


void OpenLog::setWeekday(uint8_t value)
{
	_time[TIME_DAY] = DECtoBCD(value);
	setTime(_time, TIME_ARRAY_LENGTH);
}

//Move the hours, mins, sec, etc registers from RV-3028 into the _time array
//Needs to be called before printing time or date
//We do not protect the GPx registers. They will be overwritten. The user has plenty of RAM if they need it.
void OpenLog::updateTime()
{
	readRegisterRegion(RV3028_ADDR, _time, RV3028_SECONDS, TIME_ARRAY_LENGTH);
	
	if(is12Hour()) _time[TIME_HOURS] &= ~(1<<HOURS_AM_PM); //Remove this bit from value

}

uint8_t OpenLog::getSeconds()
{
	return BCDtoDEC(_time[TIME_SECONDS]);
}

uint8_t OpenLog::getMinutes()
{
	return BCDtoDEC(_time[TIME_MINUTES]);
}

uint8_t OpenLog::getHours()
{
	return BCDtoDEC(_time[TIME_HOURS]);
}

uint8_t OpenLog::getDate()
{
	return BCDtoDEC(_time[TIME_DATE]);
}

uint8_t OpenLog::getMonth()
{
	return BCDtoDEC(_time[TIME_MONTH]);
}

uint8_t OpenLog::getYear()
{
	return BCDtoDEC(_time[TIME_YEAR]);
}

uint8_t OpenLog::getWeekday()
{
	return BCDtoDEC(_time[TIME_DAY]);
}

//Takes the time from the last build and uses it as the current time
//Works very well as an arduino sketch
void OpenLog::setToCompilerTime()
{
	_time[TIME_SECONDS] = DECtoBCD(BUILD_SECOND);
	_time[TIME_MINUTES] = DECtoBCD(BUILD_MINUTE);
	_time[TIME_HOURS] = DECtoBCD(BUILD_HOUR);

	//Build_Hour is 0-23, convert to 1-12 if needed
	if (is12Hour())
	{
		uint8_t hour = BUILD_HOUR;

		bool pm = false;

		if(hour == 0)
			hour += 12;
		else if(hour == 12)
			pm = true;
		else if(hour > 12)
		{
			hour -= 12;
			pm = true;
		}

		_time[TIME_HOURS] = DECtoBCD(hour); //Load the modified hours
	
		if(pm == true) _time[TIME_HOURS] |= (1<<HOURS_AM_PM); //Set AM/PM bit if needed
	}
	
	_time[TIME_MONTH] = DECtoBCD(BUILD_MONTH);
	_time[TIME_DATE] = DECtoBCD(BUILD_DATE);
	_time[TIME_YEAR] = DECtoBCD(BUILD_YEAR - 2000); //! Not Y2K (or Y2.1K)-proof :(
	
	// Calculate weekday (from here: http://stackoverflow.com/a/21235587)
	// 0 = Sunday, 6 = Saturday
	uint16_t d = BUILD_DATE;
	uint16_t m = BUILD_MONTH;
	uint16_t y = BUILD_YEAR;
	uint16_t weekday = (d+=m<3?y--:y-2,23*m/9+d+4+y/4-y/100+y/400)%7 + 1;
	_time[TIME_DAY] = DECtoBCD(weekday);
	
	setTime(_time, TIME_ARRAY_LENGTH);
}

void OpenLog::setAlarm(uint8_t sec, uint8_t min, uint8_t hour, uint8_t date, uint8_t month, uint8_t day)
{
	uint8_t alarmTime[TIME_ARRAY_LENGTH];
	 //This library assumes we are operating on RC oscillator. Hundredths alarm is not valid in this mode.
	alarmTime[TIME_SECONDS] = DECtoBCD(sec);
	alarmTime[TIME_MINUTES] = DECtoBCD(min);
	alarmTime[TIME_HOURS] = DECtoBCD(hour);
	alarmTime[TIME_DATE] = DECtoBCD(date);
	alarmTime[TIME_MONTH] = DECtoBCD(month);
	alarmTime[TIME_YEAR] = DECtoBCD(0); //Our alarm cannot read these values, so we set them to 0
	alarmTime[TIME_DAY] = DECtoBCD(day); //Our alarm cannot read these values, so we set them to 0

	
	setAlarm(alarmTime, TIME_ARRAY_LENGTH);
}

void OpenLog::setAlarm(uint8_t * alarmTime, uint8_t len)
{
	
	writeMultipleRegisters(RV3028_ADDR, RV3028_MINUTES_ALM, alarmTime, len);
}

/*********************************
Given a bit location, enable the interrupt
INTERRUPT_BLIE	4
INTERRUPT_TIE	3
INTERRUPT_AIE	2
INTERRUPT_EIE	1
*********************************/
void OpenLog::enableInterrupt(uint8_t source)
{
	uint8_t value = readRegister(RV3028_ADDR, RV3028_INT_MASK);
	value |= (1<<source); //Set the interrupt enable bit
	writeRegister(RV3028_ADDR, RV3028_INT_MASK, value);
}

void OpenLog::disableInterrupt(uint8_t source)
{
	uint8_t value = readRegister(RV3028_ADDR, RV3028_INT_MASK);
	value &= ~(1<<source); //Clear the interrupt enable bit
	writeRegister(RV3028_ADDR, RV3028_INT_MASK, value);
}

/********************************
Set Alarm Mode controls which parts of the time have to match for the alarm to trigger.
When the RTC matches a given time, make an interrupt fire.

Mode must be between 0 and 7 to tell when the alarm should be triggered. 
Alarm is triggered when listed characteristics match:
0: Minutes, hours, and day match (once per week)
1: Hours, and day match (once per week)
2: Minutes, and day match (once per hour for one day a week)
3: Day matches (once per week)
4: Hours and minutes match (once per day)
5: Hours match (once per day)
6: Minutes match (once per hour)
7: Disabled (defualt)
********************************/
/*void OpenLog::setAlarmMode(uint8_t mode)
{
	//set AIE in CTRL2 register to 1
	//07h-09h set MSB
	if (mode > 0b111) mode = 0b111; //0 to 7 is valid
	
	uint8_t value = readRegister(RV3028_CTDWN_TMR_CTRL);
	value &= 0b11100011; //Clear ARPT bits
	value |= (mode << 2);
	writeRegister(RV3028_CTDWN_TMR_CTRL, value);*/
//}


/*void OpenLog::setCountdownTimer(uint8_t duration, uint8_t unit, bool repeat, bool pulse)
{
	// Invalid configurations
	if (duration == 0 || unit > 0b11) {
		return;
	}

	// TIMER_VALUE_) and ! are used to control the countdown timer

	// Set timer value
	writeRegister(RV3028_CTDWN_TMR, (duration - 1));
	writeRegister(RV3028_TMR_INITIAL, (duration - 1));

	// Enable timer
	uint8_t value = readRegister(RV3028_CTDWN_TMR_CTRL);
	value &= 0b00011100; // Clear countdown timer bits while preserving ARPT
	value |= unit; // Set clock frequency
	value |= (!pulse << CTDWN_TMR_TM_OFFSET);
	value |= (repeat << CTDWN_TMR_TRPT_OFFSET);
	value |= (1 << CTDWN_TMR_TE_OFFSET); // Timer enable
	writeRegister(RV3028_CTDWN_TMR_CTRL, value);*/
//}

void OpenLog::clearInterrupts() //Read the status register to clear the current interrupt flags
{
	status();
}

uint8_t OpenLog::BCDtoDEC(uint8_t val)
{
	return ( ( val / 0x10) * 10 ) + ( val % 0x10 );
}

// BCDtoDEC -- convert decimal to binary-coded decimal (BCD)
uint8_t OpenLog::DECtoBCD(uint8_t val)
{
	return ( ( val / 10 ) * 0x10 ) + ( val % 10 );
}

void OpenLog::waitForCommandReady( void ){
  bool QOLready = false;
  while(!QOLready){   // Poll the QOL to see if it can accept a new command

    // Check the state of the cmdready register
    QOLready = readRegister(SLAVE_ADDRESS, QOL_CMDREADY_COMMAND_REG);

    if(!QOLready){
      fiber_sleep(5); // give QOL time to work in between requests (in case not done yet) (try tweaking this # if you don't like it)
    } // otherwise fall straight through
  }
}

//Send a command to the unit with options (such as "append myfile.txt" or "read myfile.txt 10")
void OpenLog::sendCommand(uint8_t registerNumber, char option1[])
{
	waitForCommandReady(); 
	char temp[strlen(option1) + 1];
	temp[0] = registerNumber;
	for (uint8_t position = 0; position < strlen(option1); position++)
	{
		temp[position + 1] = option1[position];
	}
	//temp[1] = option1[0];
	i2c.write(SLAVE_ADDRESS, temp, strlen(option1) + 1);
	//fiber_sleep(150);//Allow actions to be taken on the SD card
  //_i2cPort->beginTransmission(SLAVE_ADDRESS);
  //_i2cPort->write(registerNumber);
  /*if (option1.length() > 0)
  {
    //_i2cPort->print(" "); //Include space
    _i2cPort->print(option1);
  }*/
 
  //Upon completion any new characters sent to OpenLog will be recorded to this file
}

uint8_t OpenLog::readRegister(uint8_t address, uint8_t offset)
{
	waitForCommandReady();
	return i2c.readRegister(address, offset);
}

void OpenLog::readRegisterRegion(uint8_t address, uint8_t *outputPointer , uint8_t offset, uint8_t length)
{
	waitForCommandReady();
	i2c.readRegister(address, offset, outputPointer, length);	
}

void OpenLog::writeMultipleRegisters(uint8_t addr, uint8_t regNum, uint8_t * values, uint8_t len)
{
	char temp[I2C_BUFFER_LENGTH];
	temp[0] = regNum;  
	for (uint8_t position = 0; position < len; position++)
	{
		temp[position + 1] = values[position];
	}
	waitForCommandReady();
	i2c.write(addr, temp, len + 1);
}

void OpenLog::writeRegister(uint8_t address, uint8_t regNum, uint8_t val)
{
	waitForCommandReady();
	i2c.writeRegister(address, regNum, val);
}

//Write a single character to Qwiic OpenLog
void OpenLog::writeCharacter(uint8_t character) {
	waitForCommandReady();
  i2c.writeRegister(SLAVE_ADDRESS, LOG_WRITE_FILE, character);  
  //fiber_sleep(200);
}

void OpenLog::writeString(char *myString) {
  //_i2cPort->beginTransmission(SLAVE_ADDRESS);
  //_i2cPort->write(registerMap.writeFile);
  
  waitForCommandReady();
  char temp[I2C_BUFFER_LENGTH];
  //temp[0] = LOG_WRITE_FILE;
  temp[strlen(myString) + 1] = 0x0D;
  for (uint8_t position = 0; position < strlen(myString); position++)
  {
  	temp[position] = myString[position];
	//writeCharacter(myString[position]);
  }
  for (uint8_t position = 0; position < strlen(myString) + 1; position++)
  {
  	//temp[position] = myString[position];
	writeCharacter(temp[position]);
  }
  //fiber_sleep(50);//Allow SD card write
  //remember, the rx buffer on the i2c openlog is 32 uint8_ts
  //and the register address takes up 1 uint8_t so we can only
  //send 31 data uint8_ts at a time
  /*if(strlen(myString) > 31)
  {
    return -1;
  }
  if (strlen(myString) > 0)
  {*/
    //_i2cPort->print(" "); //Include space
    //i2c.write(SLAVE_ADDRESS, temp, strlen(myString) + 1);
  //}
}

void OpenLog::syncFile(){
  char temp[1] = {LOG_SYNC_FILE};
  i2c.write(SLAVE_ADDRESS, temp, 1);
  fiber_sleep(150);//Allow the SD card to be written
}