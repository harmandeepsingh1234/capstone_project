
/*
 * ultrasonic.c
 *
 *  Created on: Oct 28, 2020
 *      Author: Gurdeep
 */
#include <iobb.h> // Library to access GPIO of Beaglebone
#include <time.h>  //Time Library
#include <stdio.h> // Standard IO library


long int start, stop, count; // Variables to store values

clock_t before, end, difference; // To store Time of UltraSonic Sensor

int cflag = 1; // Flag to stabilize count
int cflag1 = 1; // Flag to stabilize count
int personCount = 0;

int main(void)
{
  iolib_init();  // Initialize the IOBB Library
  iolib_setdir(8, 10, DigitalOut);  // Setting Pin 10 or Port 8 as Output for Sensor -1
  iolib_setdir(8, 9, DigitalIn);    // Setting Pin 9 or Port 8 as Inputfor Sensor -1

  iolib_setdir(8, 12, DigitalOut);  // Setting Pin 12 or Port 8 as Output for Sensor -2
  iolib_setdir(8, 11, DigitalIn);    // Setting Pin 11 or Port 8 as Input for Sensor -2

  printf("STARTED \n");
  iolib_delay_ms(500); // Small Delay



  while(1)
  {
    pin_high(8, 10);  // Trigger Ultrasonic Sensor
    usleep(10);
    pin_low(8, 10);

    
    while(is_low(8, 9))  // Wait until Echo Pulse starts
    {
			count+= 1;
			before = clock(); //Current Clock Time

	}

	while(is_high(8, 9)) // Wait and store Echo pulse time
	{
		count += 0;
		end = clock();

	}

    difference = end - before; // Get Absolute time of Echo Pulse


    difference = difference/30; // Get a rough value in centimeters
    printf("Distance for entry : %d cm. \n",difference);


    if(difference < 15 && cflag == 1) // If  Ultrasonic value less than 15, (Person sensed )
    {

    	if(personCount < 10) // Limit of People Allowed Inside
    	 {
		personCount += 1; // Increase the count
		cflag = 0; // Make Flag 0 so that 1 person is not counted as many
		printf("One Person Arrived : %d \n", personCount);
		iolib_delay_ms(500); // Small Delay
		iolib_delay_ms(500);
    	 }
    	else
    	{
    		printf("Too Many People Inside, Kindly Wait \n");
			iolib_delay_ms(500);
			iolib_delay_ms(500);
			iolib_delay_ms(500);
			iolib_delay_ms(500);
    	}
	}
	if(difference > 15 && cflag == 0) // When person passes and no more in front of sensor
    {
		cflag = 1; // Make flag as 1 means ready to sense another person
	}


    pin_low(8, 10); // Make sure trigger pin is low
    iolib_delay_ms(500);  // Small delay to settle trigger and Echo
    start = 0;



    ///////////////////// OUT GOING PERSON//////////////////////////////

    pin_high(8, 12);  // Trigger Ultrasonic Sensor
    usleep(10);
    pin_low(8, 12);

    //iolib_delay_ms(1);
    while(is_low(8, 11))  // Wait until Echo Pulse starts
    {
			count+= 1;
			before = clock();

	}

	while(is_high(8, 11)) // Wait and store Echo pulse time
	{
		count += 0;
		end = clock();

	}

    difference = end - before; // Get Absolute time of Echo Pulse

    difference = difference/30; // Get a rough value in centimeters
    printf("Distnace for exit: %d cm. \n",difference);


    if(difference < 15 && cflag1 == 1) // If  Ultrasonic value less than 10, (Person sensed )
    {
    	if(personCount > 0)
    	{
		personCount -= 1; // Increase the count
		cflag1 = 0; // Make Flag 0 so that 1 Person is not counted as many
		printf("One Person Went Out : %d \n",personCount);
		iolib_delay_ms(500); // Small Delay
    	}
	}
	if(difference > 15 && cflag1 == 0) // When Person passes and no more in front of sensor
    {
		cflag1 = 1; // Make flag as 1 means ready to sense another Person
	}


    pin_low(8, 12); // Make sure trigger pin is low
    iolib_delay_ms(500);  // Small delay to settle trigger and Echo
    start = 0;

  }
  iolib_free();
  return(0);


}
