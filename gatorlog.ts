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

//% color=#f44242 icon="\uf185"
namespace gatorLog {
    // Functions for reading Particle from the gatorlog in Particle or straight adv value
	
	let commandMode = 0
	
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
		serial.readUntil("<")
		createDummyFile()
		return
	}
	
	//% weight=49
	//% blockId="gatorLog_createFile"
	//% block="create file named %value"
	export function createFile(value: string){
		command()
		serial.writeString("append " + value + String.fromCharCode(13))
		serial.readUntil("<")
		commandMode = 0;
		return
	}	
	
	function createDummyFile(){
		command()
		serial.writeString("append DELETEME.txt" + String.fromCharCode(13))
		serial.readUntil("<")
		commandMode = 0;
		return
	}
	
	function command(){
		if (commandMode == 0)
		{
			serial.writeString(String.fromCharCode(26) + String.fromCharCode(26) + String.fromCharCode(26))
			serial.readUntil(">")
			commandMode = 1
		}
		return
	}
	
	//% weight=48
	//% blockId="gatorLog_writeStringData"
	//% block="write string %value | to current file"
	export function writeStringData(value: string){
		serial.writeString(value + String.fromCharCode(13) + String.fromCharCode(10))
		commandMode = 0;
		basic.pause(20)
		return
	}
	
	//% weight=47
	//% blockId="gatorLog_mkDirectory"
	//% block="create directory with name %value""
	export function mkDirectory(value: string){
		command()
		serial.writeString("md " + value + String.fromCharCode(13))
		serial.readUntil(">")
		return
	}
	
	//% weight=46
	//% blockId="gatorLog_chDirectory"
	//% block="change to %value | directory"
	export function chDirectory(value: string){
		command()
		serial.writeString("cd " + value + String.fromCharCode(13))
		serial.readUntil(">")
		return
	}
	
	//% weight=45
	//% blockId="gatorLog_sizeOfFile"
	//% block="get size of file with name %value"
	export function sizeOfFile(value: string): string{
		command()
		serial.writeString("size ")
		serial.writeString(value)
		serial.writeString(String.fromCharCode(13))
		return serial.readString()
	}
	
	//% weight=44
	//% blockId="gatorLog_readFile"
	//% block="read file with name %value"
	export function readFile(value: string): string{
		command()
		serial.writeString("size ")
		serial.writeString(value)
		serial.writeString(String.fromCharCode(13))
		return serial.readString()
	}
	
	//% weight=42
	//% blockId="gatorLog_removeItem"
	//% block="remove file %value"
	export function removeItem(value: string){
		command()
		serial.writeString("rm ")
		serial.writeString(value)
		serial.writeString(String.fromCharCode(13))
		return
	}
	
	//% weight=41
	//% blockId="gatorLog_removeDir"
	//% block="remove directory %value | and it's contents"
	export function removeDir(value: string){
		command()
		serial.writeString("rm -rf ")
		serial.writeString(value)
		serial.writeString(String.fromCharCode(13))
		return
	}
}