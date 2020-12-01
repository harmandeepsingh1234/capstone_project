#include <stdio.h> //Standard C input Output Library
#include <unistd.h> //defines miscellaneous symbolic constants and types, and declares miscellaneous functions
#include <string.h> //C Library for various String Operations
#include <termios.h> // Contains the definitions used by the terminal I/O interfaces 
#include <sys/stat.h> //header defines the structure of the data returned by the functions fstat(), lstat(), and stat(), give file size
#include <fcntl.h> // FIle control, Open, close


 
 int main (void) // Main function
{
    int file, i; // Variable integers
    char receive[100]; // declare a char array for receiving data
    char buf[20]; // A buffer char array to store temporary data
    int bytes_read = 0;
    
    int nbytes; //size_t is an unsigned integer data type used for storing size
    int bytes_written; // //size_t is an signed integer data type used for storing size
    
    //https://linux.die.net/man/3/open
	// /dev/ttyO1 is linked with UART1 Port of Beaglebone 
    if ((file = open("/dev/ttyO1", O_RDWR | O_NOCTTY | O_NDELAY))<0) // Try opening file in Read Write mode
    {
		
        printf("UART: Failed to open the file.\n"); //A message Print
        return 0;
    }
    else
    {
    	printf("Opened UART file successfully.\n");
    }

    
    struct termios options; // the termios structure is vital
    tcgetattr(file, &options); // sets the parameters associated with file

    // Set up the communications options:
    // 9600 baud, 8-bit, enable receiver, no modem control lines
    options.c_cflag = B9600 | CS8 | CREAD | CLOCAL; //Setting Baud Rate and other Parameters
    options.c_iflag = IGNPAR | ICRNL; // ignore partity errors, CR -> newline
    tcflush(file, TCIFLUSH); // discard file information not transmitted
    tcsetattr(file, TCSANOW, &options); // changes occur immmediately

    sleep(1);
    printf("Started\n");
    
	
	while(1)
	{ 
	i = 0;

    int person=0;
    char buffer[50];

    printf("Enter Number of Person inside: ");
    scanf("%d", &person);
    sprintf(buffer, "%d", person);
    
    strcpy(buf, buffer); // Copy a string in buf char array
    nbytes = strlen(buf); // Store size of buf array in nbytes
    tcflush(file, TCIFLUSH);
    bytes_written = write(file, buf, nbytes); // Sending message to ESP
    
    printf("Sent Data to Thingspeak via UART : %d  -Bytes: %d\n",person,bytes_written);
    
    sleep(2);
	bytes_read = read(file,&receive,100); // Read the incoming Message from ESP
	// Read the file and store the data in receive , read 100 bytes max
	
	//printf("\n\nBytes Received - %d",bytes_read); // Print how many bytes was received
	printf("\n");
	
	if(bytes_read > 1) //If no. of bytes are read is more than 1
	{
		for(i=0;i<bytes_read;i++) //a for loop to print data byte by byte
		{
			printf("%c",receive[i]); //print a byte of message from ESP
		}
	printf("\n\n\n");

        sleep(5);
	}
    sleep(15);
    tcflush(file, TCIFLUSH);
	}
 
    close(file); //Close the file at last
}
