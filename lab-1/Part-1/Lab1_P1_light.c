#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

/*
 Because we are using wiringPi
 so we should use wiringPi numbering for PINs
 
    PIN name      BCM_GPIO    wiringPi
    red lED:        2         8
 yellow LED:        3         9
  green LED:        4         7
   blue LED:        5         21
 
 */
void lightAllOn(){
    // turn all LEDs ON
	digitalWrite(8,1);
        digitalWrite(9,1);
        digitalWrite(7,1);
        digitalWrite(21,1);
}
void lightAllOff(){
    // turn all LEDs OFF
	digitalWrite(8,0);
        digitalWrite(9,0);
        digitalWrite(7,0);
        digitalWrite(21,0);

}

int main(){
	// setup wiringPi
	wiringPiSetup();

	// set goal pins to operatable
	pinMode(8,OUTPUT);	// red
	pinMode(9,OUTPUT);	// yellow
	pinMode(7,OUTPUT);	// green
	pinMode(21,OUTPUT);	// blue

	// set all lights off
	lightAllOff();	
		

	/* part of here
	 * shining the light per second
	 */ 
	int timer = 0;
	int turn = 0;
	while(1){
		if(turn == 0){
            // red light ON and blue light OFF
			digitalWrite(8,1);
			digitalWrite(21,0);
			turn = 1;
		}else{
            // red light OFF and blue light ON
			digitalWrite(8,0);
			digitalWrite(21,1);
			turn = 0;	
		}
		// delay one second
		delay(1000);

	}

	return 0;
}
