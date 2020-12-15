#include <string.h> //C Library for various String Operations
#include <iobb.h>   // A header library to control GPIOs of Beaglebone
#include <time.h>  //Time Library   
#include <unistd.h> //defines miscellaneous symbolic constants and types, and declares miscellaneous functions
#include <linux/i2c-dev.h>  
#include <sys/ioctl.h>  
#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>   /* Standard input/output definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

  
#define I2C_BUS    "/dev/i2c-2" // I2C bus device on a Beaglebone Black 
#define I2C_ADDR    0x27        // I2C slave address for the LCD module  



int hexadecimal2int(char *hex); //Declaration of a function for onversion of Hexadecimal to integer
//The sensor gives data in Hexadecimal, Hence we need to convert it into Integer

void i2c_start();
void i2c_send_byte(unsigned char data);
void clear_display();
void display_on();
void go_to(int line, char position);
void send_simple_string_to_lcd(char *str);
void lcd_init();
float getTemp();
int open_port1(void);
int readBack1(void);


int i2cFile;  
int cursor_pos;  
char *i2cport = "2"; // I2c Port to which MLX90614 is connected to
char *devaddr = "0x5a"; // Address of MLX90614
char *readreg = "0x07"; // Address to Read Object Temperature

int buzzer_pin = 10;


//Variables to store values
clock_t before, end;
long int time_taken;
int distance, personCount;

int us1_trig = 8; // Ultrasonic 1 Trigger Pin
int us1_echo = 10; // Ultrasonic 1 Echo Pin

int us2_trig = 12; // Ultrasonic 2 Trigger Pin
int us2_echo = 14; // Ultrasonic 2 Echo Pin
 
int buzzerPin = 41; //Buzzer Pin
int motorEntry = 27; //Motor 1 Pin
int motorExit = 23; //Motor 2 Pin
 

int threshold = 20; //Ultrasonic Distance threshold to count
int maxPerson = 10; //Maximum allowed person
int count = 0;
int highCount = 0;

int flag1, flag2; //Variable to avoid multiple false countings

char buf[180];
int fd1;  // File descriptor
int fd4; 
int n;

  
void main() {   
	
	i2c_start();//Start I2C Communication
	
	lcd_init();//Initialize the LCD
  
	clear_display();  //Clear the display

	iolib_init();   //Initializing the iobb library

	iolib_setdir(8, buzzer_pin, DigitalOut); //Make the GPIO as output for driving Buzzer

	iolib_setdir(8, us1_trig, DigitalOut);  //Trigger pin as output
	iolib_setdir(8, us1_echo, DigitalIn);   //Echo pin as Input
	iolib_setdir(8, us2_trig, DigitalOut);  //Trigger pin as output
	iolib_setdir(8, us2_echo, DigitalIn);   //Echo pin as Input

	iolib_setdir(9, buzzerPin, DigitalOut);  //Buzzer Pin as Output
	iolib_setdir(9, motorEntry, DigitalOut); // Motor Pin as Output
	iolib_setdir(9, motorExit, DigitalOut);  // Motor Pin as Output


	// printf("BUZZER ON\n");
	// pin_high(9, buzzerPin); // Turn ON buzzer
	// pin_high(9, motorEntry); // Turn ON buzzer
	// pin_high(9, motorExit); // Turn ON buzzer
	// sleep(2);
	// pin_low(9, buzzerPin); // Turn ON buzzer
	// pin_low(9, motorExit); // Turn ON buzzer
	// pin_low(9, motorEntry); // Turn ON buzzer

	// printf("BUZZER OFF\n");
	// sleep(5);


	flag1 = 0;
	flag2 = 0;

	//Open UART 1 Port and Setting it
  fd1 = open_port1();
 
  // Read the configureation of the port 
 
  struct termios options;
  tcgetattr( fd1, &options );
 
  /* SEt Baud Rate */
  cfsetispeed( &options, B9600 );
  cfsetospeed( &options, B9600 );
  
  //I don't know what this is exactly 
  options.c_cflag |= ( CLOCAL | CREAD );
  // Set the Charactor size
  options.c_cflag &= ~CSIZE; /* Mask the character size bits */
  options.c_cflag |= CS8;    /* Select 8 data bits */
  // Set parity - No Parity (8N1)
  options.c_cflag &= ~PARENB;
  options.c_cflag &= ~CSTOPB;
  options.c_cflag &= ~CSIZE;
  options.c_cflag |= CS8;
 
  // Disable Hardware flowcontrol
 
  //  options.c_cflag &= ~CNEW_RTSCTS;  -- not supported
 
  // Enable Raw Input
  options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
  // Disable Software Flow control
  options.c_iflag &= ~(IXON | IXOFF | IXANY);
  // Chose raw (not processed) output
  options.c_oflag &= ~OPOST;

 
  if ( tcsetattr( fd1, TCSANOW, &options ) == -1 )
    printf ("Error with tcsetattr = %s\n", strerror ( errno ) );
  else
    printf ( "%s\n", "tcsetattr succeed-1" );
 
  fcntl(fd1, F_SETFL, FNDELAY);

///////////////////////////////////////////////////////////////////////




	go_to(1,0);//Go to Line 2
	send_simple_string_to_lcd("PERSON IN: 0    ");//Print a message on LCD
	go_to(2,0);//Go to Line 2
	send_simple_string_to_lcd("                ");//Print a message on LCD
	pin_low(9, motorEntry); // Turn ON buzzer
	while(1) //Infinite loop
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
					printf("Kindly Check Wiring\n"); //Show error message
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
	    
	    //printf("Sensor 1: %d cm. \n",distance);
	    
	    if(distance < threshold && flag1 == 0) // If human detected
	    {
			if(personCount >= maxPerson) //If max count reached
			{
				printf("Max Limit Reached. Kindly Wait.\n"); // Show message
				go_to(2,0);//Go to Line 2
				send_simple_string_to_lcd("Max Limit Reached.");//Print a message on LCD
				pin_high(9, buzzerPin); // Turn ON buzzer
				iolib_delay_ms(900); // delay
				iolib_delay_ms(900); // delay
				iolib_delay_ms(900); // delay
				pin_low(9, buzzerPin); // Turn ON buzzer
				
			}
			else
			{
				printf("1 Person arrived. Checking Temperature\n"); // Show message
				float objTemp = getTemp();

				printf("Detected Temperature: %.2f *C\n",objTemp);
				
				if(objTemp > 37.5) //If detected temperature is more than threshold
				{	
					
					printf("High Temperature Detected\n"); //Print a Warning message
					go_to(2,0);//Go to Line 2
					send_simple_string_to_lcd("HIGH TEMP WARNING");//Print a message on LCD
					highCount++;

	    			

					pin_high(9,buzzerPin); // Turn on Buzzer
					sleep(5);               //Keep it On for 5 seconds
					pin_low(9,buzzerPin);  //Tuen the Buzzer OFF

					go_to(2,0);//Go to Line 2
					send_simple_string_to_lcd("                ");//Print a message on LCD
				}  
				else //If temperature is normal
				{
					printf("Normal Temperature Detected. Opening Door\n"); //Print a message
					go_to(2,0);//Go to Line 2
					send_simple_string_to_lcd("OPENING DOOR..  ");//Print a message on LCD

					//pin_high(9,buzzerPin); //Turn ON Buzzer
					pin_high(9, motorEntry); // Open Entry Door
					iolib_delay_ms(500);
					iolib_delay_ms(500);
					pin_low(9, motorEntry); //Make Sure door Motor is off
	    			pin_low(9, buzzerPin); //Make Sure Buzzer Off

	    			iolib_delay_ms(500);
					iolib_delay_ms(500);

	    			go_to(2,0);//Go to Line 2
					send_simple_string_to_lcd("              ");//Print a message on LCD
				}


				
				personCount ++; //Increment count
				printf("No. of Person inside is %d. \n",personCount); //Show the PersonCount
				flag1 = 1; //
			}
			
		}
		else
		{
			flag1 = 0;
		}
	    
	    pin_low(8, us1_trig);
	    pin_low(8, us2_trig);
	    iolib_delay_ms(100);
	    

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
					printf("Kindly Check Wiring\n");
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
	    
	    //printf("Sensor 2: %d cm. \n",distance);
	    
	    if(distance < threshold && flag2 == 0  && personCount > 0 && distance > 0)
	    {
			printf("1 Person Exited. Opening Exit Gate.\n");
			go_to(2,0);//Go to Line 2
			send_simple_string_to_lcd("EXIT DOOR OPEN  ");//Print a message on LCD
			pin_high(9, motorExit);
			personCount --;
			if(personCount < 0) personCount = 0;
			printf("No. of Person inside is %d. \n",personCount);
			flag2 = 1;

			iolib_delay_ms(500); 
			iolib_delay_ms(500); 
	    	pin_low(9, motorExit);  

	    	iolib_delay_ms(500); 
			iolib_delay_ms(500);
	    	go_to(2,0);//Go to Line 2
			send_simple_string_to_lcd("              ");//Print a message on LCD
			
		}
		else
		{
			flag2 = 0;
		}

		pin_low(8, us2_trig);

		go_to(1,0);//Go to Line 2
		send_simple_string_to_lcd("PERSON IN: ");//Print a message on LCD
		char temp[10];
		char temp2[4];
		sprintf(temp,"%d",personCount);
		sprintf(temp2,"%d",highCount);
		send_simple_string_to_lcd(temp);//Print a message on LCD
		send_simple_string_to_lcd("   ");//Print a message on LCD
		strcat(temp,",");
		strcat(temp,temp2);
		printf("Uploading Data to ESP: %s \n",temp);

		n = write(fd1,temp, strlen(temp));
		if (n < 0)
		fputs("write() of bytes failed!\n", stderr);
		else
		printf ("Write succeed n = %i\n", n );
		
	    
	    
	    		 
		sleep(1);
	}
		 
	  
	close(i2cFile);  //Stop i2c communication  
} 

float getTemp()
{	
	while(1)
	{
		FILE *fp1; //File pointer to store temporary data coming from sensor
		
		//Sample Command = i2cget -y 2 0x5a 0x07 w 
		
		char command1[30] = "i2cget -y "; //Base i2c command
		strcat(command1,i2cport); //Adding Port String
		strcat(command1," "); //Adding a Space
		strcat(command1,devaddr); //I2c address of the data
		strcat(command1," "); //Adding a Space
		strcat(command1,readreg); //Register from where to read the data
		strcat(command1," w"); //Specifying write mode


		char buff1[255];//creating char array to store data of file  

		/* command contains the command string (a character array) */

		/* If you want to read output from command */
		fp1 = popen(command1,"r"); //Run command and connect the output
		/* read output from command */
		fscanf(fp1, "%s", buff1);// Read the incoming data into buff1
		fclose(fp1); //Close the File pointer 
		 

		int number1 = hexadecimal2int(buff1); //Converting incoming Hexadecimal data to Integer

		float Temp = (number1 * 0.0201) - 273.15; //Conversion of base Integer data to Float 

		if(Temp > 1000) 
		{
			sleep(2);
			continue; //Means Some Garbage date collected
		}

		return Temp;

		//printf("Object Temperature: %0.2f *C \n",objTemp); //Print the Temperature

	}

}

void i2c_start() //Function to start i2c communication
{  
   if((i2cFile = open(I2C_BUS, O_RDWR)) < 0) 
   {  
      printf("Error failed to open I2C bus [%s].\n", I2C_BUS);  
      exit(-1);  
   }  
   // set the I2C slave address for all subsequent I2C device transfers  
   if (ioctl(i2cFile, I2C_SLAVE, I2C_ADDR) < 0) 
   {  
      printf("Error failed to set I2C address [%s].\n", I2C_ADDR);  
      exit(-1);  
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
   i2c_send_byte(0b00000100); //  
   i2c_send_byte(0b00000000); // D7-D4=0  
   i2c_send_byte(0b00010100); //  
   i2c_send_byte(0b00010000); // D0=display_clear  
  
}

void display_on()
{
   /* -------------------------------------------------------------------- * 
    * Turn on the display                                                  * 
    * -------------------------------------------------------------------- */  
   usleep(40);                // wait 40usec  
   i2c_send_byte(0b00001100); //  
   i2c_send_byte(0b00001000); // D7-D4=0  
   i2c_send_byte(0b11101100); //  
   i2c_send_byte(0b11101000); // D3=1 D2=display_on, D1=cursor_on, D0=cursor_blink  
   cursor_pos =1 ;  
}   
  
  
void go_to(int line, char position)  //Function to move cursor of lcd
{  
	if(line==1)  position = 0x80+position;  

	else if(line==2)  position = 0xC0+position;  
		  
	int i;  
	
	char n1,n2,ln1,ln2,mask1,mask2;  
	   
	ln1 = 0b11111100;  
	ln2 = 0b11111000;  
	    
	mask1 = 0b11110000;  
	mask2 = 0b00001111;  
	    
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
			
		ln1 = 0b11111101;  
		ln2 = 0b11111001;  
			
		mask1 = 0b11110000;  
		mask2 = 0b00001111;  
		  
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
   i2c_send_byte(0b00110100); // D7=0, D6=0, D5=1, D4=1, RS,RW=0 EN=1  
   i2c_send_byte(0b00110000); // D7=0, D6=0, D5=1, D4=1, RS,RW=0 EN=0  
  
   sleep(0.1);                // wait 10msec  
   i2c_send_byte(0b00110100); //   
   i2c_send_byte(0b00110000); // same  
   sleep(0.1);                // wait 10msec  
   i2c_send_byte(0b00110100); //  
   i2c_send_byte(0b00110000); // 8-bit mode init complete  
   sleep(0.1);                // wait 10msec  
   i2c_send_byte(0b00100100); //  
   i2c_send_byte(0b00100000); // switched now to 4-bit mode  
  
  
   /* -------------------------------------------------------------------- * 
    * 4-bit mode initialization complete. Now configuring the function set * 
    * -------------------------------------------------------------------- */  
   usleep(100);               // wait 100usec  
   i2c_send_byte(0b00100100); //  
   i2c_send_byte(0b00100000); // keep 4-bit mode  
   i2c_send_byte(0b10000100); //  D3=2lines
   i2c_send_byte(0b10000000); // D2=char5x8  
  
  
  
   clear_display();
  
  
	display_on();
  
    sleep(1); //delay of 1 second	
}

int hexadecimal2int(char *hdec)  //Function to Convert Hexadecimal to Integer

{
    int finalval = 0;
    while (*hdec) 
    {
        
        int onebyte = *hdec++; 
        
        if (onebyte >= '0' && onebyte <= '9')
        {
			onebyte = onebyte - '0';
		}
        else if (onebyte >= 'a' && onebyte <='f') {onebyte = onebyte - 'a' + 10;}
        else if (onebyte >= 'A' && onebyte <='F') {onebyte = onebyte - 'A' + 10;}  
        
        finalval = (finalval << 4) | (onebyte & 0xF);
    }
    finalval = finalval - 524288;
    return finalval;
}

int open_port1(void)
{
  int fd; /* File descriptor for the port */
 
  fd = open("/dev/ttyO1", O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd == -1)
    {
      perror("open_port: Unable to open /dev/O1 - ");
    }
  else
    fcntl(fd, F_SETFL, FNDELAY);
 
  printf ( "In Open port fd = %i\n", fd);
 
  return (fd);
}

int readBack1(void)
{
 char buff;
  n = read( fd1, &buf, 80 );

  printf("Received Bytes Port 1: %d \n",n);
  int i = 0;
  for(i =0; i < n;i++)
  {
    //printf ( "Buf %d = %c\n",i, buf[i] );
    printf("%c",buf[i]);
  }
  printf("\n");
  return 1;
}

