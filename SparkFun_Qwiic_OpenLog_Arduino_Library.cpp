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
MicroBit uBit;

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

using namespace std;
//Attempt communication with the device
//Return true if we got a 'Polo' back from Marco
void OpenLog::begin(uint8_t deviceAddress)
{
  _deviceAddress = deviceAddress; //If provided, store the I2C address from user

  //We require caller to begin their I2C port, with the speed of their choice
  //external to the library
  //_i2cPort->begin();

  //Check communication with device
  getStatus();
}

//Get the version number from OpenLog
ManagedString OpenLog::getVersion()
{
  uint8_t versionMajor = readRegister(_deviceAddress, LOG_FWMAJOR); 
  uint8_t versionMinor = readRegister(_deviceAddress, LOG_FWMINOR); 

  return(ManagedString(versionMajor) + "." + ManagedString(versionMinor));
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

  return readRegister(_deviceAddress, LOG_STATUS);
}

//Change the I2C address of the OpenLog
//This will be recorded to OpenLog's EEPROM and config.txt file.
bool OpenLog::setI2CAddress(uint8_t addr)
{
  ManagedString temp;
  temp = addr;
  bool result = sendCommand(LOG_I2CADDRESS, temp);

  //Upon completion any new communication must be with this new I2C address  

  _deviceAddress = addr; //Change the address internally

  return(result);
}

//Append to a given file. If it doesn't exist it will be created
bool OpenLog::append(ManagedString fileName)
{
  return (sendCommand(LOG_OPEN_FILE, fileName));
  //Upon completion any new characters sent to OpenLog will be recorded to this file
}

//Create a given file in the current directory
bool OpenLog::create(ManagedString fileName)
{
  return (sendCommand(LOG_CREATE_FILE, fileName));
  //Upon completion a new file is created but OpenLog is still recording to original file
}

//Given a directory name, create it in whatever directory we are currently in
bool OpenLog::makeDirectory(ManagedString directoryName)
{
  return (sendCommand(LOG_MKDIR, directoryName));
  //Upon completion Qwiic OpenLog will respond with its status
  //Qwiic OpenLog will continue logging whatever it next receives to the current open log
}

//Given a directory name, change to that directory
bool OpenLog::changeDirectory(ManagedString directoryName)
{
  return (sendCommand(LOG_CD, directoryName));
  //Upon completion Qwiic OpenLog will respond with its status
  //Qwiic OpenLog will continue logging whatever it next receives to the current open log
}

//Return the size of a given file. Returns a 4 uint8_t signed long
int32_t OpenLog::size(ManagedString fileName)
{
  sendCommand(LOG_FILE_SIZE, fileName);
  //Upon completion Qwiic OpenLog will have 4 uint8_ts ready to be read

  //_i2cPort->requestFrom(_deviceAddress, (uint8_t)4);

  uint8_t data[4];
  readRegisterRegion(_deviceAddress, data, LOG_FILE_SIZE, 4);
  int32_t fileSize = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];

  return (fileSize);
}

//Read the contents of a file, up to the size of the buffer, into a given array, from a given spot
void OpenLog::read(uint8_t* userBuffer, uint16_t bufferSize, ManagedString fileName)
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

    /*_i2cPort->requestFrom(_deviceAddress, toGet);
    while (_i2cPort->available())
      userBuffer[spotInBuffer++] = _i2cPort->read();
	*/
	readRegisterRegion(_deviceAddress, &userBuffer[spotInBuffer], LOG_READ_FILE, toGet);
    leftToRead -= toGet;
	spotInBuffer += toGet;
  }
}

//Read the contents of a directory. Wildcards allowed
//Returns true if OpenLog ack'd. Use getNextDirectoryItem() to get the first item.
bool OpenLog::searchDirectory(ManagedString options)
{
  if (sendCommand(LOG_LIST, options) == true)
  {
    _searchStarted = true;
    return (true);
    //Upon completion Qwiic OpenLog will have a file name or directory name ready to respond with, terminated with a \0
    //It will continue to respond with a file name or directory until it responds with all 0xFFs (end of list)
  }
  return (false);
}

//Returns the name of the next file or directory folder in the current directory
//Returns "" if it is the end of the list
ManagedString OpenLog::getNextDirectoryItem()
{
  if (_searchStarted == false) return (""); //We haven't done a search yet

  std::vector<char> itemName;
  //_i2cPort->requestFrom(_deviceAddress, (uint8_t)I2C_BUFFER_LENGTH);
  char tempData;
  //uint8_t charsReceived = 0;
  while (tempData != '\0')
  {
	uBit.i2c.read(_deviceAddress, &tempData, 1, true);
	itemName.push_back(tempData);
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
  char itemNameArray[itemName.size()];
  for (uint8_t copy = 0; copy < itemName.size(); copy++)
  {
	itemNameArray[copy] = itemName[copy];
  }
  ManagedString itemNameToReturn(itemNameArray);
  //We shouldn't get this far but if we do
  return(itemNameToReturn);
}

//Remove a file, wildcards supported
//OpenLog will respond with the number of items removed
uint32_t OpenLog::removeFile(ManagedString thingToDelete)
{
	return(remove(thingToDelete, false));
}

//Remove a directory, wildcards supported
//OpenLog will respond with 1 when removing a directory
uint32_t OpenLog::removeDirectory(ManagedString thingToDelete)
{
	return(remove(thingToDelete, true)); //Delete all files in the directory as well
}

//Remove a file or directory (including everything in that directory)
//OpenLog will respond with the number of items removed
//Returns 1 if only a directory is removed (even if directory had files in it)
uint32_t OpenLog::remove(ManagedString thingToDelete, bool removeEverything)
{
  char data[4];
  if(removeEverything == true)
  {
	sendCommand(LOG_RMRF, thingToDelete); //-rf causes any directory to remove contents as well
    uBit.i2c.read(_deviceAddress, data, 4);
  }
  else
  {
	sendCommand(LOG_RM, thingToDelete); //Just delete a thing
    uBit.i2c.read(_deviceAddress, data, 4);
  }
  //Upon completion Qwiic OpenLog will have 4 uint8_ts ready to read, representing the number of files beleted

  int32_t filesDeleted = 0;
  
  filesDeleted = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];

  return (filesDeleted); //Return the number of files removed

  //Qwiic OpenLog will continue logging whatever it next receives to the current open log
}

//Send a command to the unit with options (such as "append myfile.txt" or "read myfile.txt 10")
bool OpenLog::sendCommand(uint8_t registerNumber, ManagedString option1)
{
	char temp[option1.length() + 1];
	temp[0] = registerNumber;
	for (int position = 0; position < option1.length(); position++)
	{
		temp[position + 1] = option1.charAt(position);
	}
	//temp[1] = option1[0];
	uBit.i2c.write(_deviceAddress, temp, option1.length() + 1, false);
  //_i2cPort->beginTransmission(_deviceAddress);
  //_i2cPort->write(registerNumber);
  /*if (option1.length() > 0)
  {
    //_i2cPort->print(" "); //Include space
    _i2cPort->print(option1);
  }*/
 
  return (true);
  //Upon completion any new characters sent to OpenLog will be recorded to this file
}

uint8_t OpenLog::readRegister(uint8_t address, uint8_t offset)
{
	return uBit.i2c.readRegister(address, offset);
}

void OpenLog::readRegisterRegion(uint8_t address, uint8_t *outputPointer , uint8_t offset, uint8_t length)
{
	uBit.i2c.readRegister(address, offset, outputPointer, length);	
}

//Write a single character to Qwiic OpenLog
size_t OpenLog::write(uint8_t character) {
  uBit.i2c.writeRegister(_deviceAddress, LOG_WRITE_FILE, character);
  return (1);
}

int OpenLog::writeString(ManagedString myString) {
  //_i2cPort->beginTransmission(_deviceAddress);
  //_i2cPort->write(registerMap.writeFile);
  char temp[I2C_BUFFER_LENGTH];
  temp[0] = LOG_WRITE_FILE;
  for (int position = 0; position < myString.length(); position++)
  {
  	temp[position + 1] = myString.charAt(position);
  }
  //remember, the rx buffer on the i2c openlog is 32 uint8_ts
  //and the register address takes up 1 uint8_t so we can only
  //send 31 data uint8_ts at a time
  if(myString.length() > 31)
  {
    return -1;
  }
  if (myString.length() > 0)
  {
    //_i2cPort->print(" "); //Include space
    uBit.i2c.write(_deviceAddress, temp, (uint8_t)myString.length());
  }

  return (1);
}

bool OpenLog::syncFile(){
  uBit.i2c.write(_deviceAddress, (char *)LOG_SYNC_FILE, 1);
  /*_i2cPort->beginTransmission(_deviceAddress);
  _i2cPort->write(registerMap.syncFile);
  
  if (_i2cPort->endTransmission() != 0){
    return (0);    
  }
  */

  return (1);
}