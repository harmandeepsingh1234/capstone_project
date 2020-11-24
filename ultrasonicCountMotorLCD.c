#include <iobb.h> // library for gpio pins of BBB
#include <time.h>  //Time Library   
#include <stdio.h> //standard input output library
#include <unistd.h> //defines miscellaneous symbolic constants and types, and declares miscellaneous functions
#include <stdlib.h>
#include <linux/i2c-dev.h>  
#include <fcntl.h>  //File Control - To access I2C File
#include <sys/ioctl.h>  //input ouput control, control input and output to a file
  
#define I2C_BUS    "/dev/i2c-2" // I2C bus device on a Beaglebone Black 
#define I2C_ADDR    0x27        // I2C slave address for the LCD module  

int i2cFile;  //For storing i2c file location
int cursor_pos;  //To store cursor position


//Variables to store values
clock_t before, end;
long int time_taken;
int distance, personCount;

int us1_trig = 8; // Ultrasonic 1 Trigger Pin
int us1_echo = 10; // Ultrasonic 1 Echo Pin

int us2_trig = 12; // Ultrasonic 2 Trigger Pin
int us2_echo = 14; // Ultrasonic 2 Echo Pin
 
int buzzerPin = 23; //Buzzer Pin
int motorEntry = 25; //Motor 1 Pin
int motorExit = 27; //Motor 2 Pin
 

int threshold = 20; //Ultrasonic Distance threshold to count
int maxPerson = 10; //Maximum allowed person


int flag1, flag2; //Variable to avoid multiple false countings
int count;

void i2c_start() //Function to start i2c communication
{  
   if((i2cFile = open(I2C_BUS, O_RDWR)) < 0) //Trying to open i2c file in read write mode 
   {  
      printf("Error failed to open I2C bus [%s].\n", I2C_BUS);  

   }  
   // set the I2C slave address for all subsequent I2C device transfers  
   if (ioctl(i2cFile, I2C_SLAVE, I2C_ADDR) < 0) //Starting i2c control on address of LCD screen
   {  
      printf("Error failed to set I2C address [%s].\n", I2C_ADDR);  

   }  
}  
  
  
  
void i2c_send_byte(unsigned char data) //Sens a byte of data via i2c 
{  
   unsigned char byte[1];  
   byte[0] = data;  
   write(i2cFile, byte, sizeof(byte));   
   /* -------------------------------------------------------------------- * 
    * Below wait creates 1msec delay, needed by display to catch commands  * 
    * -------------------------------------------------------------------- */  
   usleep(1000);  
}  
  

void clear_display()  //Function to clear Display
{  
   /* -------------------------------------------------------------------- * 
    * Display clear, cursor home                                           * 
    * -------------------------------------------------------------------- */  
   usleep(40);                // wait 40usec  
   i2c_send_byte(0b00000100); //  0x04 // increment cursur
   i2c_send_byte(0b00000000); //  0
   i2c_send_byte(0b00010100); //  0x14// move cursur right
   i2c_send_byte(0b00010000); //  0x10// move cursur left
  
}

void display_on()
{
   /* -------------------------------------------------------------------- * 
    * Turn on the display                                                  * 
    * -------------------------------------------------------------------- */  
   usleep(40);                // wait 40usec  
   i2c_send_byte(0b00001100); //  0x0C //// display on, cursor off
   i2c_send_byte(0b00001000); //  0x08// clear display without clearing DDRAM
   i2c_send_byte(0b11101100); //  0xEC
   i2c_send_byte(0b11101000); // 0xE8
   cursor_pos =1 ;  
}   
  
  
void go_to(int line, char position)  //Function to move cursor of lcd
{  
	if(line==1)  position = 0x80+position;  //Predefined position for line 1  // set cursor to row 1, column 1

	else if(line==2)  position = 0xC0+position;  //Predefined position for line 2 0xC0
		  
	int i;  
	
	char n1,n2,ln1,ln2,mask1,mask2;  

	//byte - 8bits
	//We are operating the screen in 4 bit mode
	//4 bit - nibble
	   
	ln1 = 0b11111100;  //0xFC//Command for lcd to know that incoming data is Command
	ln2 = 0b11111000;  //0xF8//Command for lcd to know that incoming data is command
	    
	mask1 = 0b11110000; //0xF0
	mask2 = 0b00001111;  //0x0F
	    
	n1 = (position & mask1) | mask2;        
	n2 = ((position & mask2)<<4) | mask2;  
	   
	i2c_send_byte( n1 & ln1) ;  
	i2c_send_byte( n1 & ln2) ;  
	i2c_send_byte( n2 & ln1) ;  
	i2c_send_byte( n2 & ln2) ;  
	
	
}  
  
  
  
  
void send_simple_string_to_lcd(char *str) //A function to send Simple String to LCD 
{  
	int i;  
	for(i=0;(str[i]!=0x00);i++)  
		{  
		char chr = str[i];
		int i;  
		char n1,n2,ln1,ln2,mask1,mask2;  

		ln1 = 0b11111101;  //Command for lcd to know that incoming data is String
		ln2 = 0b11111001;  //Command for lcd to know that incoming data is String

		//12345678
		//00001111
		//00005678
		//56781111    //12341111
//ln1   //11111101 ln2//11111001
		//5678 1101   //1234 1001

		//1001 in end means it is starting part of message which needs to be shown
		//1101 in end means it is ending part of message to to be shown

		//1234 5678

		mask1 = 0b11110000;  //Mask to get start 4 bit of the character
		mask2 = 0b00001111;  //Mask to get end 4 bit of the character
		  
		n1 = (chr & mask1)| mask2;  
		n2 =  ((chr & mask2)<<4)|mask2;
			
		i2c_send_byte( n1 & ln1) ; 
		i2c_send_byte( n1 & ln2);      
		i2c_send_byte( n2 & ln1) ;  
		i2c_send_byte( n2 & ln2) ;  
		}  
	  
  
}  

void lcd_init()
{
   /* -------------------------------------------------------------------- * 
    * Initialize the display, using the 4-bit mode initialization sequence * 
    * -------------------------------------------------------------------- */
   sleep(0.4);                // wait 40msec  
   i2c_send_byte(0b00110100); //0x34// D7=0, D6=0, D5=1, D4=1, RS,RW=0 EN=1
   i2c_send_byte(0b00110000); // 0x30// D7=0, D6=0, D5=1, D4=1, RS,RW=0 EN=0  //8-bit mode
  
   sleep(0.1);                // wait 10msec  
   i2c_send_byte(0b00110100); //0x34
   i2c_send_byte(0b00110000); // same  
   sleep(0.1);                // wait 10msec  
   i2c_send_byte(0b00110100); //  0x34
   i2c_send_byte(0b00110000); // 8-bit mode init complete  
   sleep(0.1);                // wait 10msec  
   i2c_send_byte(0b00100100); //  0x24
   i2c_send_byte(0b00100000); // 0x24// switched now to 4-bit mode
   /* -------------------------------------------------------------------- * 
    * 4-bit mode initialization complete. Now configuring the function set * 
    * -------------------------------------------------------------------- */  
   usleep(100);               // wait 100usec  
   i2c_send_byte(0b00100100); //  0x24
   i2c_send_byte(0b00100000); // 0x20//keep 4-bit mode
   i2c_send_byte(0b10000100); //  0x84//D3=2lines
   i2c_send_byte(0b10000000); // 0x80//D2=char5x8
   clear_display();
	display_on();
    sleep(1); //delay of 1 second	
}
  


int main(void)  
{  
	i2c_start();//Start I2C Communication

	lcd_init();//Initialize the LCD

	clear_display();  //Clear the display

  iolib_init();  //Initialize iobb library
  
  iolib_setdir(8, us1_trig, DigitalOut);  //Trigger pin as output
  iolib_setdir(8, us1_echo, DigitalIn);   //Echo pin as Input

  iolib_setdir(8, us2_trig, DigitalOut);  //Trigger pin as output
  iolib_setdir(8, us2_echo, DigitalIn);   //Echo pin as Input
  
  iolib_setdir(9, buzzerPin, DigitalOut);  //Buzzer Pin as Output
  iolib_setdir(9, motorEntry, DigitalOut); // Motor Pin as Output
  iolib_setdir(9, motorExit, DigitalOut);  // Motor Pin as Output
  
  flag1 = 0;
  flag2 = 0;
  
  go_to(1,0);//Go to Line 1
  	send_simple_string_to_lcd("PERSON IN: 0    ");//Print a message on LCD
  	go_to(2,0);//Go to Line 2
  	send_simple_string_to_lcd("                ");//Print a message on LCD

  while(1)  
  {  
	//Trigerring Ultrasonic - 1
    pin_high(8, us1_trig);   //Generate Ultrasonic Pulse
    usleep(10); 
    pin_low(8, us1_trig); 

    

    while(is_low(8, us1_echo)) //Wait for Echo pin to get ready
    {		
			count+= 1; //Kep loop count for checking possible error
			before = clock(); //Store current clock time
			
			if(count > 5000) //If no response for long time
			{
				printf("Kindly Check Wiring - 1\n"); //Show error message
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
    distance = time_taken/30; //Calculate distance
    
    //Rough formula to calculate distance from time
    //


    //printf("%d cm. \n",distance);
    
    if(distance < threshold && flag1 == 0) // If human detected
    {
		if(personCount >= maxPerson) //If max count reached
		{
			printf("Max Limit Reached. Kindly Wait.\n"); // Show message

			go_to(2,0);//Go to Line 2
			send_simple_string_to_lcd("Max Limit Reached.");//Print a message on LCD

			pin_high(9, buzzerPin); // Turn ON buzzer
			iolib_delay_ms(900); // delay
			
		}
		else
		{
			printf("1 Person arrived. Opening Entry Gate\n"); // Show message
			pin_high(9, motorEntry); // Open Entry Door
			personCount ++; //Increment count
			printf("No. of Person inside is %d. \n",personCount); //Show the PersonCount
			flag1 = 1; //
			go_to(2,0);//Go to Line 2
			send_simple_string_to_lcd("                ");//Print a message on LCD
		}
		
	}
	else
	{
		flag1 = 0;
	}
    
    pin_low(8, us1_trig);
    iolib_delay_ms(500);
    pin_low(9, motorEntry); //Make Sure door Motor is off
    pin_low(9, buzzerPin); //Make Sure Buzzer Off



	//Trigerring Ultrasonic - 2
    pin_high(8, us2_trig);  //Generate Ultrasonic Pulse
    usleep(10); 
    pin_low(8, us2_trig); 
    
    while(is_low(8, us2_echo))
    {		
			count+= 1;
			before = clock();
			
			if(count > 5000)
			{
				printf("Kindly Check Wiring-2\n");
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
    
    //printf("%d cm. \n",distance);
    
    if(distance < threshold && flag2 == 0)
    {
		printf("1 Person Exited. Opening Exit Gate.\n");
		pin_high(9, motorExit);
		personCount --;
		if(personCount < 0) personCount = 0;
		printf("No. of Person inside is %d. \n",personCount);
		flag2 = 1;
		go_to(2,0);//Go to Line 2
		send_simple_string_to_lcd("                ");//Print a message on LCD
		
	}
	else
	{
		flag2 = 0;
	}

	go_to(1,0);//Go to Line 2
	send_simple_string_to_lcd("PERSON IN: ");//Print a message on LCD
	char temp[3];
	sprintf(temp,"%d",personCount);//Converting Integer count to char array(string)
	send_simple_string_to_lcd(temp);//Print a message on LCD
	send_simple_string_to_lcd("   ");//Print a message on LCD
	
    
    pin_low(8, us2_trig);
    iolib_delay_ms(500); 
    pin_low(9, motorExit);  
      
  }  
  iolib_free();  
  return(0);  
    
      
  
}  


