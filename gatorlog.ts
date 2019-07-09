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
	
	//% weight=50 
	//% blockId="gatorLog_begin" 
	//% block="initialize gator:log"
	export function begin(){
		serial.redirect(SerialPin.P15, SerialPin.P14, BaudRate.BaudRate9600)
		return
	}
	
	//% weight=49
	//% blockId="gatorLog_createFile"
	//% block="create file named %value"
	export function createFile(value: string){
		serial.writeString(",,,new ")
		serial.writeLine(value)
		return
	}
	
	//% weight=48
	//% blockId="gatorLog_writeStringData"
	//% block="write string %value | to current file"
	export function writeStringData(value: string){
		serial.writeLine(value)
		return
	}
	
	//% weight=47
	//% blockId="gatorLog_mkDirectory"
	//% block="create directory with name %value""
	export function mkDirectory(value: string){
		serial.writeString(",,,md ")
		serial.writeLine(value)
		return
	}
	
	//% weight=46
	//% blockId="gatorLog_chDirectory"
	//% block="change to %value | directory"
	export function chDirectory(value: string){
		serial.writeString(",,,cd ")
		serial.writeLine(value)
		return
	}
	
	//% weight=45
	//% blockId="gatorLog_sizeOfFile"
	//% block="get size of file with name %value"
	export function sizeOfFile(value: string): string{
		serial.writeString(",,,size ")
		serial.writeLine(value)
		return serial.readString()
	}
	
	//% weight=44
	//% blockId="gatorLog_readFile"
	//% block="get size of file with name %value"
	export function readFile(value: string): string{
		serial.writeString(",,,size ")
		serial.writeLine(value)
		return serial.readString()
	}
	
	//% weight=42
	//% blockId="gatorLog_removeItem"
	//% block="remove file %value"
	//% shim=gatorLog::removeItem
	export function removeItem(value: string){
		serial.writeString(",,,rm ")
		serial.writeLine(value)
		return
	}
	
	//% weight=41
	//% blockId="gatorLog_removeDir"
	//% block="remove directory %value | and it's contents"
	//% shim=gatorLog::removeDir
	export function removeDir(value: string){
		serial.writeLine(value)
		serial.writeString(",,,rm -rf ")
		return
	}
}