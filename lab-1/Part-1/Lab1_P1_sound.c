#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

void biu(int *freq){
	/*
	 for this function, 
	 set the signal of pin 22 to High
	 then delay it for giving time for processing data
	 set the sighal of pin 22 to Low
	 then delay it for giving time for processing data
		figure is like:
		high(wait) low (wait) hight (wait) low (wait)
		-----------|__________|------------|__________
-	*/
	digitalWrite(22, 1);
	delay(*freq);
	digitalWrite(22, 0);
	delay(*freq);
}

// Set All Button as OFF
void init(){
	pullUpDnControl(27, PUD_OFF);
	pullUpDnControl(0, PUD_OFF);
	pullUpDnControl(1, PUD_OFF);
	pullUpDnControl(24, PUD_OFF);
	pullUpDnControl(28, PUD_OFF);
}



int main(){
	// setup for wiringPi
	wiringPiSetup();
	// setup pins
	pinMode(27,INPUT);	// btn1
	pinMode(0,INPUT);	// btn2
	pinMode(1,INPUT);	// btn3
	pinMode(24,INPUT);	// btn4
	pinMode(28,INPUT);	// btn5
	pinMode(22, OUTPUT);	// spkr

	// main
	int input;
	int pin_num;
	int freq;
	int mode;
	
	// set enter
	while(1){
		printf("Enter 0 for normal sound, Enter 1 for fancy sound:\n");
		scanf("%d",&mode);
		printf("Enter the Button Number: \n");
		scanf("%d",&input);
		if ((input >= 1 && input <= 5) && (mode == 0 || mode == 1)){
			break;		
		}
	}
	//init
	init();
	// change the input number to pin number and set the frequency of wave
	switch(input){
		case 1:// button 1
			pin_num = 27;
			freq = 100;
			break;
		case 2:// button 2
			pin_num = 0;
			freq = 50;
			break;
		case 3:// button 3
			pin_num = 1;
			freq = 30;
			break;
		case 4:// button 4
			pin_num = 24;
			freq = 15;
			break;
		case 5:// button 5
			pin_num = 28;
			freq = 1;
			break;
		default:
			break;
	}

	// mode 0 works well and making an ugly sound :)
	if (mode == 0){
		while(1){
			if(digitalRead(pin_num)){
				// set the pin on, so the speaker will be sounding without push button
				pullUpDnControl(pin_num, PUD_UP);
				biu(&freq);
			
			}
			
		}
    // mode 1 supposed to making a nice alarm sound, but does not implement well yet.
	} else if (mode == 1){
		while(1){
			if(digitalRead(pin_num)){
				// set the pin on, so the speaker will be sounding without push button
				pullUpDnControl(pin_num, PUD_UP);
				break;
			}
		}
		int s;
        /*
         for the alarm sound,
         the tone goes from high frequency to low frequency
         */
		for (s = 1; s <1000; s++){
			if (s >= 995){
				s = 1;
			}
			biu(&s);	
		}
	} else {
		printf("you are amazing!");	
	}
	
	

	return 0;
}
