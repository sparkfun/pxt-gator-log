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

#pragma once


#include "mbed.h"
#include "MicroBit.h"
#include "ManagedString.h"


#define QOL_CMDREADY_COMMAND_REG  (0x12)
#define QOL_CONTINUEREADFILE_COMMAND_REG  (0x13)


//Bits found in the getStatus() uint8_t
#define STATUS_SD_INIT_GOOD 0
#define STATUS_LAST_COMMAND_SUCCESS 1
#define STATUS_LAST_COMMAND_KNOWN 2
#define STATUS_FILE_OPEN 3
#define STATUS_IN_ROOT_DIRECTORY 4
  
//Platform specific configurations

//Define the size of the I2C buffer based on the platform the user has
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//The catch-all default is 32

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


#define TIME_ARRAY_LENGTH 7 // Total number of writable values in device

enum time_order {
	TIME_SECONDS,    // 0
	TIME_MINUTES,    // 1
	TIME_HOURS,      // 2
	TIME_DAY,
	TIME_DATE,       // 3
	TIME_MONTH,      // 4
	TIME_YEAR,       // 5
};

class OpenLog{

  public:
    //These functions override the built-in print functions so that when the user does an 
    //myLogger.println("send this"); it gets chopped up and sent over I2C instead of Serial
    void writeCharacter(uint8_t character);
    void writeString(char *myString);
    void syncFile(void);

    //By default use the default I2C addres, and use Wire port
    void begin();

    uint16_t getVersion(); //Returns a char *that is the current firmware version
    uint8_t getStatus(); //Returns various status bits

    bool setI2CAddress(char addr); //Set the I2C address we read and write to
    void append(char *fileName); //Open and append to a file
    void create(char *fileName); //Create a file but don't open it for writing
    void makeDirectory(char *directoryName); //Create the given directory
    void changeDirectory(char *directoryName); //Change to the given directory
    int32_t size(char *fileName); //Given a file name, read the size of the file

    void read(uint8_t* userBuffer, uint16_t bufferSize, char *fileName); //Read the contents of a file into the provided buffer

    bool searchDirectory(char *options); //Search the current directory for a given wildcard
    void getNextDirectoryItem(uint8_t* userBuffer); //Return the next file or directory from the search

    uint32_t removeFile(char *thingToDelete); //Remove file
    uint32_t removeDirectory(char *thingToDelete); //Remove a directory including the contents of the directory
    uint32_t remove(char *thingToDelete, bool removeEverthing); //Remove file or directory including the contents of the directory

	//RV3028 Functions
	void setTime(uint8_t sec, uint8_t min, uint8_t hour, uint8_t date, uint8_t month, uint8_t year, uint8_t day);
	void setTime(uint8_t * time, uint8_t len);
	void setSeconds(uint8_t value);
	void setMinutes(uint8_t value);
	void setHours(uint8_t value);
	void setDate(uint8_t value);
	void setWeekday(uint8_t value);
	void setMonth(uint8_t value);
	void setYear(uint8_t value);
	
	void updateTime(); //Update the local array with the RTC registers
	
	char* stringDateUSA(); //Return date in mm-dd-yyyy
	char* stringDate(); //Return date in dd-mm-yyyy
	char* stringTime(); //Return time hh:mm:ss with AM/PM if in 12 hour mode
	char* stringTimeStamp(); //Return timeStamp in ISO 8601 format yyyy-mm-ddThh:mm:ss
	
	uint8_t getSeconds();
	uint8_t getMinutes();
	uint8_t getHours();
	uint8_t getDate();
	uint8_t getWeekday();
	uint8_t getMonth();
	uint8_t getYear();	
	
	void setToCompilerTime(); //Uses the hours, mins, etc from compile time to set RTC
	
	bool is12Hour(); //Returns true if 12hour bit is set
	bool isPM(); //Returns true if is12Hour and PM bit is set
	void set12Hour();
	void set24Hour();
	
	uint8_t status(); //Returns the status byte
	
	void setAlarm(uint8_t sec, uint8_t min, uint8_t hour, uint8_t date, uint8_t month, uint8_t day);
	void setAlarm(uint8_t * time, uint8_t len);
	//void setAlarmMode(uint8_t mode); //0 to 7, alarm goes off with match of second, minute, hour, etc
 	
	//void setCountdownTimer(uint8_t duration, uint8_t unit, bool repeat = true, bool pulse = true);

	void enableInterrupt(uint8_t source); //Enables a given interrupt within Interrupt Enable register
	void disableInterrupt(uint8_t source); //Disables a given interrupt within Interrupt Enable register
	void enableAlarmInterrupt(); //Use in conjuction with setAlarm and setAlarmMode
	
	void clearInterrupts();
		
	//Values in RTC are stored in Binary Coded Decimal. These functions convert to/from Decimal
	uint8_t BCDtoDEC(uint8_t val); 
	uint8_t DECtoBCD(uint8_t val);

	void reset(void); //Fully reset RTC to all zeroes
    //These are the core functions that send a command to OpenLog
    void sendCommand(uint8_t registerNumber, char option1[]);
    void waitForCommandReady( void );
	uint8_t readRegister(uint8_t address, uint8_t offset);
    void readRegisterRegion(uint8_t address, uint8_t *outputPointer , uint8_t offset, uint8_t length);
 // Reads up to one full I2C buffer from the QOL - sends a command first with the register number and 

    void writeRegister(uint8_t addr, uint8_t regNum, uint8_t val);
	void writeMultipleRegisters(uint8_t addr, uint8_t regNum, uint8_t * values, uint8_t len);
  private:

	uint8_t _time[TIME_ARRAY_LENGTH];
    //Variables
    uint8_t _deviceAddress; //Keeps track of I2C address. setI2CAddress changes this.
    uint8_t _escapeCharacter = 26; //The character that needs to be sent to QOL to get it into command mode
    uint8_t _escapeCharacterCount = 3; //Number of escape characters to get QOL into command mode

    bool _searchStarted = false; //Goes true when user does a search. Goes false when we reach end of directory.
};