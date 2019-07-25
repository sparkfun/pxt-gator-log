/**
* Andy England @ SparkFun Electronics
* September 6, 2018
* Development environment specifics:
* Written in Microsoft Makecode
* Tested with a SparkFun gatorlog sensor and micro:bit
*
* This code is released under the [MIT License](http://opensource.org/licenses/MIT).
* Please review the LICENSE.md file included with this example. If you have any questions
* or concerns with licensing, please contact techsupport@sparkfun.com.
* Distributed as-is; no warranty is given.
*/


/**
 * Functions to operate the gatorlog sensor
 */

 enum returnDataType{
	ASCII=1,
	HEXADECIMAL=2,
	RAW=3
 }

//% color=#f44242 
//% icon="\uf0ce"
namespace gatorLog {
    // Functions for reading Particle from the gatorlog in Particle or straight adv value
	
	let commandMode = 0
	let currentFile = ""
	let carriageReturn = String.fromCharCode(13)
	let newLine = String.fromCharCode(10)
	let commandReady = ">"
	let writeReady = "<"
		
	//The only reason we have this function is so we don't set currentFile to DELETEME.txt
	function dummyFile(){
		command()
		serial.writeString("append DELETEME.txt" + carriageReturn)
		serial.readUntil(writeReady)
		basic.pause(20)
		commandMode = 0;
		removeItem("DELETEME.txt")
		return
	}
	
	function command(){
		if (commandMode == 0)
		{
			serial.writeString(String.fromCharCode(26) + String.fromCharCode(26) + String.fromCharCode(26))
			serial.readUntil(commandReady)
			basic.pause(20)
			commandMode = 1
		}
		return
	}
	
	/**
	* Initializes gator:log and waits until it says it is ready to be written to.
	*/
	//% weight=50 
	//% blockId="gatorLog_begin" 
	//% block="initialize gator:log"
	export function begin(){
		basic.pause(2500)
		serial.redirect(SerialPin.P15, SerialPin.P14, BaudRate.BaudRate9600)
		pins.digitalWritePin(DigitalPin.P13, 1)
		basic.pause(100)
		pins.digitalWritePin(DigitalPin.P13, 0)
		basic.pause(100)
		pins.digitalWritePin(DigitalPin.P13, 1)
		serial.readUntil(writeReady)
		basic.pause(20)
		dummyFile()
		return
	}
	
	/**
	* Opens the file with the name provided (don't forget to provide an extension). If the file does not exist, it is created.
	*/
	//% weight=49
	//% blockId="gatorLog_openFile"
	//% block="open file named %value"
	export function openFile(value: string){
		command()
		serial.writeString("append " + value + carriageReturn)
		serial.readUntil(writeReady)
		basic.pause(20)
		currentFile = value
		commandMode = 0;
		return
	}
	
	/**
	* Removes the file with the provided name
	*/
	//% weight=48
	//% blockId="gatorLog_removeItem"
	//% block="remove file %value"
	export function removeItem(value: string){
		command()
		serial.writeString("rm " + value + carriageReturn)
		serial.readUntil(commandReady)
		basic.pause(20)
		return
	}
	
	/**
	* Creates a folder. Note that this block does not open the folder that it creates
	*/
	//% weight=47
	//% blockId="gatorLog_mkDirectory"
	//% block="create folder with name %value"
	export function mkDirectory(value: string){
		command()
		serial.writeString("md " + value + carriageReturn)
		serial.readUntil(commandReady)
		basic.pause(20)
		return
	}
	
	/**
	* Opens a folder. Note that the folder must already exist on your SD card. To go back to the root/home folder, use "Change to '..' folder"
	*/
	//% weight=46
	//% blockId="gatorLog_chDirectory"
	//% block="change to %value | folder"
	export function chDirectory(value: string){
		command()
		serial.writeString("cd " + value + carriageReturn)
		serial.readUntil(commandReady)
		basic.pause(20)
		return
	}
	
	/**
	* Removes a folder
	*/
	//% weight=45
	//% blockId="gatorLog_removeDir"
	//% block="remove folder %value | and it's contents"
	export function removeDir(value: string){
		command()
		serial.writeString("rm -rf " + value + carriageReturn)
		serial.readUntil(commandReady)
		basic.pause(20)
		return
	}
	
	/**
	* Writes a line of text to the current open file. If no file has been opened, this will be recorded to the LOGxxxx.txt folder
	*/
	//% blockId="gatorLog_writeLine"
	//% weight=44
	//% block="write line %value | to current file"
	export function writeLine(value: string){
		if (commandMode == 1)
		{
			serial.writeString("append " + currentFile + carriageReturn)
			serial.readUntil(writeReady)
			basic.pause(20)
		}
		serial.writeString(value + carriageReturn + newLine)
		commandMode = 0
		basic.pause(20)
		return
	}
	
	/**
	* Writes text to the current open file. If no file has been opened, this will be recorded to the LOGxxxx.txt folder
	
	//% blockId="gatorLog_writeText"
	//% weight=43
	//% block="write %value | to current file"
	export function writeText(value: string){
		if (commandMode == 1)
		{
			serial.writeString("append " + currentFile + carriageReturn)
			serial.readUntil(writeReady)
			basic.pause(20)
		}
		serial.writeString(value)
		commandMode = 0
		basic.pause(20)
		return
	}*/
	
	/**
	* Writes text to the current open file at the position specified. If no file has been opened, this will be recorded to the LOGxxxx.txt folder
	
	//% weight=42
	//% blockId="gatorLog_writeLineOffset"
	//% block="write line %value | at position %offset"
	export function writeLineOffset(value: string, offset: number){
		command()
		serial.writeString("write " + currentFile + " " + String(offset) + carriageReturn)
		serial.readUntil(writeReady)
		basic.pause(20)
		serial.writeString(value + carriageReturn + newLine)
		serial.readUntil(writeReady)
		basic.pause(20)
		serial.writeString(carriageReturn + newLine)
		serial.readUntil(commandReady)
		basic.pause(20)
		return
	}*/
	
	/**
	* Read contents of the file with the specified name
	
	//% weight=41
	//% blockId="gatorLog_readFile"
	//% block="read file with name %value"
	export function readFile(value: string): string{
		command()
		serial.writeString("read " + value + carriageReturn)
		serial.readUntil(newLine)//Use this and the readUntil(commandReady) to properly frame the openLogs response
		let returnString = serial.readUntil(carriageReturn)
		serial.readUntil(commandReady)
		basic.pause(20)
		return returnString
	}*/
	
	/**
	* Read data starting at the given position from the file with the specified name.
	
	//% weight=40
	//% blockId="gatorLog_readFileOffset"
	//% block="read from %value | starting at position %offset"
	export function readFileOffset(value: string, offset: number): string{
		command()
		serial.writeString("read " + value + " " + String(offset) + carriageReturn)
		serial.readUntil(newLine)//Use this and the readUntil(commandReady) to properly frame the openLogs response
		let returnString = serial.readUntil(carriageReturn)
		serial.readUntil(commandReady)
		basic.pause(20)
		return returnString
	}*/
	
	/**
	* Read a length of data starting at the given position from the file with the specified name.
	
	//% weight=39
	//% blockId="gatorLog_readFileOffsetLength"
	//% block="read %length | characters from %value | starting at %offset"
	export function readFileOffsetLength(value: string, length: number, offset: number): string{
		command()
		serial.writeString("read " + value + " " + String(offset) + " " + String(length) + carriageReturn)
		serial.readUntil(newLine)//Use this and the readUntil(commandReady) to properly frame the openLogs response
		let returnString = serial.readUntil(carriageReturn)
		serial.readUntil(commandReady)
		basic.pause(20)
		return returnString
	}*/
	
	/**
	* Read a length of data starting at the given position from the file with the specified name. Type determines what format the data is returned in.
	
	//% weight=38
	//% blockId="gatorLog_readFileOffsetLengthType"
	//% block="read %length | data from %value | starting at %offset | in output type %returnDataType"
	export function readFileOffsetLengthType(value: string, length: number, offset: number, type: returnDataType): string{
		command()
		serial.writeString("read " + value + " " + String(offset) + " " + String(length) + " " + String(returnDataType) + carriageReturn)
		serial.readUntil(newLine)//Use this and the readUntil(commandReady) to properly frame the openLogs response
		let returnString = serial.readUntil(carriageReturn)
		serial.readUntil(commandReady)
		basic.pause(20)
		return returnString
	}*/
	
	/**
	* Returns the size of the specified file
	
	//% weight=37
	//% blockId="gatorLog_sizeOfFile"
	//% block="get size of file with name %value"
	export function sizeOfFile(value: string): string{
		command()
		serial.writeString("size " + value + carriageReturn)
		serial.readUntil(newLine)//Use this and the readUntil(commandReady) to properly frame the openLogs response
		let returnString = serial.readUntil(carriageReturn)
		serial.readUntil(commandReady)
		basic.pause(20)
		return returnString
	}*/
}