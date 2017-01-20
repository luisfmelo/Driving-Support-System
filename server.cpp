#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <fstream>
#include <strings.h>
#include <stdlib.h>
#include <string>
#include <pthread.h>
using namespace std;

void *task1(void *);

static int connFd;

/*
 * pId: process Id - identifica a thred
 */
int main(int argc, char* argv[]) {
    int pId, portNo, listenFd;
    socklen_t len;  //store size of the address
    bool loop = false;
    struct sockaddr_in svrAdd, clntAdd;

    pthread_t threadA[3];

    if (argc < 2)
    {
        cerr << "Comando a Executar: ./server <port>" << endl;
        return 0;
    }

    portNo = atoi(argv[1]); // vai buscar a port

    if( (portNo > 65535) || (portNo < 2000) )
    {
        cerr << "Please enter a port number between 2000 - 65535" << endl;
        return 0;
    }

    //create socket
    listenFd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(listenFd < 0)
    {
        cerr << "Cannot open socket" << endl;
        return 0;
    }

    bzero((char*) &svrAdd, sizeof(svrAdd));

    svrAdd.sin_family = AF_INET;
    svrAdd.sin_addr.s_addr = INADDR_ANY;
    svrAdd.sin_port = htons(portNo);

    int yes = 1;
    /* Evita ter de estar à espera pelo timeout do socket */
    if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        cout << "Error setsockopt" << endl;
        exit(-1);
    }

    //bind socket
    if(bind(listenFd, (struct sockaddr *)&svrAdd, sizeof(svrAdd)) < 0)
    {
        cerr << "Cannot bind" << endl;
        return 0;
    }

    listen(listenFd, 5);

    int noThread = 0;

    while (noThread < 3){
        len = sizeof(clntAdd);

        cout << "Listening" << endl;

        //this is where client connects. svr will hang in this mode until client conn
        connFd = accept(listenFd, (struct sockaddr *)&clntAdd, &len);

        if (connFd < 0)
        {
            cerr << "Cannot accept connection" << endl;
            return 0;
        }
        else
            cout << "Connection successful" << endl;

            //        pthread_t thread;
        pthread_create(&threadA[noThread], NULL, task1, &connFd);
        //pthread_create(&threadA[//  pthread_join(thread, NULL);
        //pthread_join(threadA[noThread], NULL);

        noThread++;
    }


    for(int i = 0; i < 3; i++)
        cout << pthread_join(threadA[i], NULL) << endl;



}

void *task1 (void *dummyPt){
    char test[300];
    bzero(test, 301);
    int id = connFd;
    cout << "Thread No: " << pthread_self() << ". Está no socket " << id << "." << endl;
    while(1)
    {
        bzero(test, 301);
        read(id, test, 300);

        string tester (test);
        cout << "Socket " << id << " said: " << tester << endl;

        if(tester == "exit")
            break;
    }
    cout << "\nClosing thread and conn" << endl;
    close(id);
}
