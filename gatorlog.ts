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
}