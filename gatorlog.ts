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

//% color=#f44242 icon="\uf185"
namespace gatorLog {
    // Functions for reading Particle from the gatorlog in Particle or straight adv value
	
	let commandMode = 0
	let currentFile = ""
	let carriageReturn = String.fromCharCode(13)
	let newLine = String.fromCharCode(10)
	let commandReady = ">"
	let writeReady = "<"
	
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
		dummyFile()
		return
	}
	
	//% weight=49
	//% blockId="gatorLog_openFile"
	//% block="open file named %value"
	export function openFile(value: string){
		command()
		serial.writeString("append " + value + carriageReturn)
		serial.readUntil(writeReady)
		currentFile = value
		commandMode = 0;
		return
	}	
	
	//The only reason we have this function is so we don't set currentFile to DELETEME.txt
	function dummyFile(){
		command()
		serial.writeString("append DELETEME.txt" + carriageReturn)
		serial.readUntil(writeReady)
		commandMode = 0;
		removeItem("DELETEME.txt")
		return
	}
	
	function command(){
		if (commandMode == 0)
		{
			serial.writeString(String.fromCharCode(26) + String.fromCharCode(26) + String.fromCharCode(26))
			serial.readUntil(commandReady)
			commandMode = 1
		}
		return
	}
	
	//% blockId="gatorLog_writeStringData"
	//% weight=48
	//% block="write line %value | to current file"
	export function writeStringData(value: string){
		if (commandMode == 1)
		{
			serial.writeString("append " + currentFile + carriageReturn)
			serial.readUntil(writeReady)
		}
		serial.writeString(value + carriageReturn + newLine)
		commandMode = 0
		basic.pause(20)
		return
	}
	
	//% weight=30
	//% blockId="gatorLog_writeStringDataOffset"
	//% block="write line %value | at position %offset"
	//% advanced=true
	export function writeStringDataOffset(value: string, offset: number){
		command()
		serial.writeString("write " + currentFile + " " + String(offset) + carriageReturn)
		serial.readUntil(writeReady)
		serial.writeString(value + carriageReturn + newLine)
		serial.readUntil(writeReady)
		serial.writeString(carriageReturn + newLine)
		serial.readUntil(commandReady)
		basic.pause(20)
		return
	}
	
	//% weight=47
	//% blockId="gatorLog_mkDirectory"
	//% block="create directory with name %value""
	export function mkDirectory(value: string){
		command()
		serial.writeString("md " + value + carriageReturn)
		serial.readUntil(commandReady)
		return
	}
	
	//% weight=46
	//% blockId="gatorLog_chDirectory"
	//% block="change to %value | directory"
	export function chDirectory(value: string){
		command()
		serial.writeString("cd " + value + carriageReturn)
		serial.readUntil(commandReady)
		return
	}
	
	//% weight=45
	//% blockId="gatorLog_sizeOfFile"
	//% block="get size of file with name %value"
	export function sizeOfFile(value: string): string{
		command()
		serial.writeString("size " + value + carriageReturn)
		serial.readUntil(newLine)//Use this and the readUntil(commandReady) to properly frame the openLogs response
		let returnString = serial.readUntil(carriageReturn)
		serial.readUntil(commandReady)
		return returnString
	}
	
	//% weight=44
	//% blockId="gatorLog_readFile"
	//% block="read file with name %value"
	export function readFile(value: string): string{
		command()
		serial.writeString("read " + value + carriageReturn)
		serial.readUntil(newLine)//Use this and the readUntil(commandReady) to properly frame the openLogs response
		let returnString = serial.readUntil(carriageReturn)
		serial.readUntil(commandReady)
		return returnString
	}
	
	//% weight=29
	//% blockId="gatorLog_readFileOffset"
	//% block="read from file with name %value | starting at position %offset"
	//% advanced=true
	export function readFileOffset(value: string, offset: number): string{
		command()
		serial.writeString("read " + value + " " + String(offset) + carriageReturn)
		serial.readUntil(newLine)//Use this and the readUntil(commandReady) to properly frame the openLogs response
		let returnString = serial.readUntil(carriageReturn)
		serial.readUntil(commandReady)
		return returnString
	}
	
	//% weight=28
	//% blockId="gatorLog_readFileOffsetLength"
	//% block="read %length | characters from file with name %value | starting at position %offset"
	//% advanced=true
	export function readFileOffsetLength(value: string, length: number, offset: number): string{
		command()
		serial.writeString("read " + value + " " + String(offset) + " " + String(length) + carriageReturn)
		serial.readUntil(newLine)//Use this and the readUntil(commandReady) to properly frame the openLogs response
		let returnString = serial.readUntil(carriageReturn)
		serial.readUntil(commandReady)
		return returnString
	}
	
	//% weight=27
	//% blockId="gatorLog_readFileOffsetLengthType"
	//% block="read %length | characters from file with name %value | starting at position %offset | in output type %returnDataType"
	//% advanced=true
	export function readFileOffsetLengthType(value: string, length: number, offset: number, type: returnDataType): string{
		command()
		serial.writeString("read " + value + " " + String(offset) + " " + String(length) + " " + String(returnDataType) + carriageReturn)
		serial.readUntil(newLine)//Use this and the readUntil(commandReady) to properly frame the openLogs response
		let returnString = serial.readUntil(carriageReturn)
		serial.readUntil(commandReady)
		return returnString
	}
	
	//% weight=42
	//% blockId="gatorLog_removeItem"
	//% block="remove file %value"
	export function removeItem(value: string){
		command()
		serial.writeString("rm " + value + carriageReturn)
		serial.readUntil(commandReady)
		return
	}
	
	//% weight=41
	//% blockId="gatorLog_removeDir"
	//% block="remove directory %value | and it's contents"
	export function removeDir(value: string){
		command()
		serial.writeString("rm -rf " + value + carriageReturn)
		serial.readUntil(commandReady)
		return
	}
}