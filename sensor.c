#include <stdio.h> //Standard C input Output Library
#include <string.h> //C Library for various String Operations
#include <unistd.h> //LIBRARY TO USE STANDARD FUNCTIONS LIKE SLEEP
#include <iobb.h>   // A header library to control GPIOs of Beaglebone

int hexadecimal2int(char *hex); //Declaration of a function for on version of Hexadecimal to integer


////////// PIN CONNECTION ////////
//MLX90614         BBB
// VCC            VDD_5V
// GND            D_GND
// SCL            I2C2_SCL(P9 19)
// SDA            I2C2_SDA(P9 20)
//Buzzer GND(-)    -  DGND
//Buzzer VCC(+)    -  P8 10


//The sensor gives data in Hexadecimal, Hence we need to convert it into Integer
//char *i2cport = "2"; // I2c Port to which MLX90614 is connected to
//char *devaddr = "0x5a"; // Address of MLX90614 as per datasheet
//char *readreg = "0x07"; // Address to Read Object Temperature as per the datasheet


int buzzer_pin = 10;

void main() // Main Function
{
	iolib_init();   //Initializing the iobb library

	iolib_setdir(8, buzzer_pin, DigitalOut); //Make the GPIO as output for driving Buzzer

	while(1) //Infinite loop

	{
		FILE *fp1; //File pointer to store temporary data coming from sensor

		//Sample Command = i2cget -y 2 0x5a 0x07 w
		char command1[30] = "i2cget -y 2 0x5a 0x07 w"; //Base i2c command

		char buff1[255];//creating char array to store data of file

		//popen executes the command on terminal and gives the output
		fp1 = popen(command1,"r"); //Run command and connect the output

		/* read output from command */
		fscanf(fp1, "%s", buff1);// Read the incoming data into buff1

		fclose(fp1); //Close the File pointer

		int number1 = hexadecimal2int(buff1); //Converting incoming Hexadecimal data to Integer

		float objTemp = (number1 * 0.0201) - 273.15; //Conversion of base Integer data to Float

		printf("Object Temperature: %0.2f *C \n",objTemp); //Print the Temperature

		if(objTemp > 1000)
			{
				continue; //Means Some Garbage date collected
			}
		else if (objTemp < -70)// If temperature below -70 which is out of Sensor Range
				{
					printf("Corrupted Data Received. Kindly Check the Sensor Wiring.\n"); //Print a Warning message
				}
		else if(objTemp > 37.5) //If detected temperature is more than threshold
		{
			printf("High Temperature Detected\n"); //Print a Warning message
			pin_high(8,buzzer_pin); // Turn on Buzzer
			sleep(5);               //Keep it On for 5 seconds
			pin_low(8,buzzer_pin);  //Tuen the Buzzer OFF
		}
		else //If temperature is normal
		{
			printf("Normal Temperature Detected\n"); //Print a message
			pin_high(8,buzzer_pin); //Turn ON Buzzer
			sleep(0.5);             //For half second for indication
			pin_low(8,buzzer_pin);  //Turn Off the Buzzer
		}
		sleep(1);
	}
}
int hexadecimal2int(char *hdec)  //Function to Convert Hexadecimal to Integer

{
    int finalval = 0; // set final value to 0
    while (*hdec) // set a pointer
    {
        int onebyte = *hdec++; // increment the pointer

        if (onebyte >= '0' && onebyte <= '9')  // if a byte has address between 0 and 9
        {
			onebyte = onebyte - '0'; // then substract the value of 0 from the onebyte
	}
        else if (onebyte >= 'a' && onebyte <='f')  // if a byte has address between a and f
        {
        	onebyte = onebyte - 'a' + 10;  // then substract the value of a from the onebyte and add 10
        }
        else if (onebyte >= 'A' && onebyte <='F') // if a byte has address between A and F
        {
        	onebyte = onebyte - 'A' + 10;  // then substract the value of A from the onebyte and add 10
        }
        finalval = (finalval << 4) | (onebyte & 0xF); // OR operation to get final value
    }
    finalval = finalval - 524288; // Substract offset value
    return finalval;  // return final value
}
