#include <stdio.h>    // Standard input/output definitions 
#include <stdlib.h> 
#include <stdint.h>   // Standard types 
#include <string.h>   // String function definitions 
#include <unistd.h>   // UNIX standard function definitions 
#include <fcntl.h>    // File control definitions 
#include <errno.h>    // Error number definitions 
#include <termios.h>  // POSIX terminal control definitions 
#include <sys/ioctl.h>
#include <getopt.h>

#define BAUD B9600
#define PORT "/dev/ttyACM0"

/*
 * Open port PORT (define)
 * return:
 * file descriptor fd: in case of success
 * -1 in case of error
 */
int init_serialport();


/*
 * Read Serial Port until terminator char - until
 * return:
 * buf: buffer with text
 * 0 in case of success
 */
int serialport_read_until(int fd, char* buf, char until);
