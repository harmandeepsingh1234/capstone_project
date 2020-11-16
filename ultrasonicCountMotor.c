#include <iobb.h> 
#include <time.h>  //Time Library   
#include <stdio.h>
#include <unistd.h> //defines miscellaneous symbolic constants and types, and declares miscellaneous functions


//Variables to store values
clock_t before, end;
long int time_taken;
int distance, personCount;

int us1_trig = 8; // Ultrasonic 1 Trigger Pin
int us1_echo = 10; // Ultrasonic 1 Echo Pin

int us2_trig = 12; // Ultrasonic 2 Trigger Pin
int us2_echo = 14; // Ultrasonic 2 Echo Pin
 
int motorEntry_open = 15;  //Motor 1 Pin - IN1
int motorEntry_close = 23; //Motor 1 Pin - IN2
int motorExit_open = 25;  //Motor 2 Pin  - IN3
int motorExit_close = 27; //Motor 2 Pin - IN4
 
int threshold = 20; //Ultrasonic Distance threshold to count
int maxPerson = 10; //Maximum allowed person


int flag1, flag2; //Variable to avoid multiple false countings of person
int count;  //variable for checking possible error

int main(void)  
{  
  iolib_init();  //Initialize iobb library
  
  iolib_setdir(8, us1_trig, DigitalOut);  //Trigger pin as output
  iolib_setdir(8, us1_echo, DigitalIn);   //Echo pin as Input
  iolib_setdir(8, us2_trig, DigitalOut);  //Trigger pin as output
  iolib_setdir(8, us2_echo, DigitalIn);   //Echo pin as Input
  
  iolib_setdir(9, motorEntry_open, DigitalOut); // Motor1 Pin as Output
  iolib_setdir(9, motorExit_open, DigitalOut);  // Motor2 Pin as Output
  iolib_setdir(9, motorEntry_close, DigitalOut); // Motor1 Pin as Output
  iolib_setdir(9, motorExit_close, DigitalOut);  // Motor2 Pin as Output

  flag1 = 0;
  flag2 = 0;
  
  while(1)  
  {  
	//Trigerring Ultrasonic - 1
    pin_high(8, us1_trig);   //Generate Ultrasonic Pulse
    usleep(10); 
    pin_low(8, us1_trig); 
    
    while(is_low(8, us1_echo)) //Wait for Echo pin to get ready
    {		

			before = clock(); //Store current clock time
			
			count+= 1; //Kep loop count for checking possible error
			if(count > 5000) //If no response for long time
			{
				printf("Connection error\n"); //Show error message
				count = 0;
				break;
			}
			
			
	}
	count = 0;
	
	while(is_high(8, us1_echo)) //Wait for Echo Pin to go Low - means signal received
	{	
		end = clock();	//Store Current Clock
	}
    
    time_taken = end - before;
    distance = time_taken/30; //Calculate distance as sound travels at 343 m/s so it needs 29.155 microseconds/cm so round off to 30
    
    
    if(distance < threshold && flag1 == 0) // If human detected
    {
		if(personCount >= maxPerson) //If max count reached
		{
			printf("Max Limit Reached. Kindly Wait.\n"); // Show message
			
		}
		else
		{
			printf("1 Person arrived. Opening Entry Gate\n"); // Show message
			personCount ++; //Increment count
			printf("No. of Person inside is %d. \n",personCount); //Show the PersonCount
			flag1 = 1; //

			pin_high(9, motorEntry_open); // Open Entry Door
			sleep(5);
			pin_low(9, motorEntry_open); //Make Sure door Motor is off

			pin_high(9, motorEntry_close); // Closing Entry Door
			sleep(5);
			pin_low(9, motorEntry_close);


		}
		
	}
	else
	{
		flag1 = 0;
	}
    
    pin_low(8, us1_trig);
    iolib_delay_ms(500);


	//Trigerring Ultrasonic - 2
    pin_high(8, us2_trig);  //Generate Ultrasonic Pulse
    usleep(10); 
    pin_low(8, us2_trig); 
    
    while(is_low(8, us2_echo))  //Wait for Echo pin to get ready
    {		
			count+= 1;
			before = clock();  //Store current clock time
			
			if(count > 5000)
			{
				printf("Connection error\n"); //Show error message
				count = 0;
				break;
			}
			
			
	}
	count = 0;
	
	while(is_high(8, us2_echo))
	{	
		end = clock();	
	}
    
    time_taken = end - before;
    distance = time_taken/30;
    
    
    if(distance < threshold && flag2 == 0)
    {
		printf("1 Person Exited. Opening Exit Gate.\n"); //show message
		personCount --;
		if(personCount < 0) personCount = 0;
		printf("No. of Person inside is %d. \n",personCount);
		flag2 = 1;
		
		pin_high(9, motorExit_open); // Open Exit Door
		sleep(5);
		pin_low(9, motorExit_open); //make sure door motor is off

		pin_high(9, motorExit_close); //closing exit door
		sleep(5);
		pin_low(9, motorExit_close);



	}
	else
	{
		flag2 = 0;  //flag o means ready to sense another person
	}
    
    pin_low(8, us2_trig); //make sure trigger pin is low
    iolib_delay_ms(500);  //small delay to settle trigger and echo pins
      
  }  
  iolib_free();  //free up the library resources
  return(0);  
    
      
  
}  


