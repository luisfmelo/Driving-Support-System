#include "lib_equipaD.h"
#include <string.h>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <strings.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <vector>
using namespace std;

/*
 * gcc client.c lib_equipaD.o
 * ./client <host name> <port>
 */
int main(int argc, char *argv[]) 
{
	int fd = 0;
	char buf[256];
	int rc,n;
	int listenFd, portNo;

  struct sockaddr_in svrAdd;
  struct hostent *server;

  if(argc < 3)
  {
      cerr<<"Syntax : ./client <host name> <port>"<<endl;
      return 0;
  }

/************************************************************************/
/************************************************************************/
/************************** CLIENT TCP SETUP  ***************************/
/************************************************************************/
/************************************************************************/

  portNo = atoi(argv[2]);

  if((portNo > 65535) || (portNo < 2000))
  {
      cerr<<"Please enter port number between 2000 - 65535"<<endl;
      return 0;
  }

  //create client skt
//    listenFd = socket(AF_INET, SOCK_STREAM, 0);
  listenFd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  if(listenFd < 0)
  {
      cerr << "Cannot open socket" << endl;
      return 0;
  }

  server = gethostbyname(argv[1]);

  if(server == NULL)
  {
      cerr << "Host does not exist" << endl;
      return 0;
  }

  bzero((char *) &svrAdd, sizeof(svrAdd));
  svrAdd.sin_family = AF_INET;

  bcopy((char *) server -> h_addr, (char *) &svrAdd.sin_addr.s_addr, server -> h_length);

  svrAdd.sin_port = htons(portNo);

  int checker = connect(listenFd,(struct sockaddr *) &svrAdd, sizeof(svrAdd));

  if (checker < 0)
  {
      cerr << "Cannot connect!" << endl;
      return 0;
  }


/************************************************************************/
/************************************************************************/
/************************* SERIAL PORT SETUP  ***************************/
/************************************************************************/
/************************************************************************/

    
	fd = init_serialport();
	if(fd==-1) return -1;
    	rc = 0;
	while(1) {
		if (rc == 0){
			buf[0] = '\0';
			rc = serialport_read_until(fd, buf, '.');
			printf("%s", buf);
			if(buf[0] != '\0') 
				write(listenFd, buf, strlen(buf));

			usleep( 100 * 1000 ); // wait 100 msec try again
		}
	}

	exit(EXIT_SUCCESS);    
}
