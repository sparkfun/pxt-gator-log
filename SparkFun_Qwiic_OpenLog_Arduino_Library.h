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

class OpenLog{

  public:
    //These functions override the built-in print functions so that when the user does an 
    //myLogger.println("send this"); it gets chopped up and sent over I2C instead of Serial
    virtual size_t writeCharacter(uint8_t character);
    int writeString(char *myString);
    bool syncFile(void);

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

    //These are the core functions that send a command to OpenLog
    void sendCommand(uint8_t registerNumber, char *option1);
	uint8_t readRegister(uint8_t address, uint8_t offset);
    void readRegisterRegion(uint8_t address, uint8_t *outputPointer , uint8_t offset, uint8_t length);
  private:

    //Variables
    uint8_t _deviceAddress; //Keeps track of I2C address. setI2CAddress changes this.
    uint8_t _escapeCharacter = 26; //The character that needs to be sent to QOL to get it into command mode
    uint8_t _escapeCharacterCount = 3; //Number of escape characters to get QOL into command mode

    bool _searchStarted = false; //Goes true when user does a search. Goes false when we reach end of directory.
};