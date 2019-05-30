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
	
	//% weight=31 
	//% blockId="gatorLog_begin" 
	//% block="Initialize gator:log"
	//% shim=gatorLog::begin
	export function begin(){
		return
	}
	
	//% weight=30
	//% blockId="gatorLog_createFile"
	//% block="Create file named %value"
	//% shim=gatorLog::createFile
	export function createFile(value: string){
		return
	}
	
	//% weight=30
	//% blockId="gatorLog_openFile"
	//% block="Open file %value"
	//% shim=gatorLog::openFile
	export function openFile(value: string){
		return
	}
	
	//% weight=30
	//% blockId="gatorLog_writeStringData"
	//% block="Write string %value | to current file"
	//% shim=gatorLog::writeStringData
	export function writeStringData(value: string){
		return
	}
	
	//% weight=30
	//% blockId="gatorLog_writeNumberData"
	//% block="Write number %value | to current file"
	export function writeNumberData(value: number){
		writeStringData(value.toString())
		return
	}
	
	//% weight=30
	//% blockId="gatorLog_mkDirectory"
	//% block="Create directory with name %value""
	//% shim=gatorLog::mkDirectory
	export function mkDirectory(value: string){
		return
	}
	
	//% weight=30
	//% blockId="gatorLog_chDirectory"
	//% block="Change to %value | directory"
	//% shim=gatorLog::chDirectory
	export function chDirectory(value: string){
		return
	}
	
	//% weight=30
	//% blockId="gatorLog_sizeOFile"
	//% block="Get size of file with name %value"
	//% shim=gatorLog::sizeOfFile
	export function sizeOfFile(value: string): number{
		return 0
	}
	
	//% weight=30
	//% blockId="gatorLog_search"
	//% block="Search current directory for %value"
	//% shim=gatorLog::search
	export function search(value: string): boolean{
		return true;
	}
	
	//% weight=30
	//% blockId="gatorLog_getNextItem"
	//% block="Get name of next item"
	//% shim=gatorLog::getNextItem
	export function getNextItem(): string{
		return "item";
	}
	
	//% weight=30
	//% blockId="gatorLog_removeItem"
	//% block="Remove file %value"
	//% shim=gatorLog::removeItem
	export function removeItem(value: string){
		return
	}
	
	//% weight=30
	//% blockId="gatorLog_removeDir"
	//% block="Remove directory %value | and it's contents"
	//% shim=gatorDir::removeDir
	export function removeDir(value: string){
		return
	}
}