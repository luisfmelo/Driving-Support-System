#include "lib_equipaD.h"

int init_serialport()
{
	struct termios toptions;
	int fd;

	fd = open(PORT, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1)  {
		perror("INIT: Unable to open port ");
		return -1;
	}
    
	if (tcgetattr(fd, &toptions) < 0) {
		perror("INIT: Couldn't get term attributes");
		return -1;
	}
	speed_t brate = BAUD; // let you override switch below if needed


/* Set custom options */

	// set Baud Rate
	cfsetispeed(&toptions, brate);
	cfsetospeed(&toptions, brate);

	// 8N1
	toptions.c_cflag &= ~PARENB;
	toptions.c_cflag &= ~CSTOPB;
	toptions.c_cflag &= ~CSIZE;
	toptions.c_cflag |= CS8;
	// no flow control
	toptions.c_cflag &= ~CRTSCTS;

	toptions.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
	toptions.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl

	toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
	toptions.c_oflag &= ~OPOST; // make raw

	// see: http://unixwiz.net/techtips/termios-vmin-vtime.html
	toptions.c_cc[VMIN]  = 0;
	toptions.c_cc[VTIME] = 20;
    
	if( tcsetattr(fd, TCSANOW, &toptions) < 0) {
		perror("INIT: Couldn't set term attributes");
		return -1;
	}

	return fd;
}







int serialport_read_until(int fd, char* buf, char until)
{
	char b[1];
	int i=0;
	do { 
		int n = read(fd, b, 1);  // read a char at a time
		if( n==-1) continue;   	 // couldn't read
		if( n== 0 ) {
			usleep( 1000 ); // wait 1 msec try again
			continue;
		}
		buf[i] = b[0];
		i++;
	} while( b[0] != until );

	buf[i] = 0;  // null terminate the string
	return 0;
}


