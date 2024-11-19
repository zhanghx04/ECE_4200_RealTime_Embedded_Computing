#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <softTone.h>
#include <stdbool.h>
#include <unistd.h>



int main(){
	wiringPiSetup();
	pinMode(22, PWM_OUTPUT);
	pwmSetMode(PWM_MODE_BAL);
	
	int n = 220;
	while(true){
		pwmWrite(22,n);
		sleep(0.00001);
		n += 114;
	}
	// setup for wiringPi
/*
	wiringPiSetup();
	
	// setup buttons
	pinMode(27,INPUT);	// btn1
	pinMode(0,INPUT);	// btn2
	pinMode(1,INPUT);	// btn3
	pinMode(24,INPUT);	// btn4
	pinMode(28,INPUT);	// btn5
	// setup speaker
	pinMode(22,PWM_OUTPUT);	// speaker
	
	while(true){
		pwmWrite(22, 700);
		sleep(100);
		
	}

	wiringPiSetupGpio();
	pinMode(6,PWM_OUTPUT);
	analogWrite(6,127);
	pwmWrite(6,0);
	
	//
	int i = 0;
	while(i<1000){
		printf("#\n");
		pwmWrite(6,i);
		//sleep(0.013);
		delay(13);
		i++;
	}
*/
	return 0;
}
