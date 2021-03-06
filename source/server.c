#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "controller.h"

#define BUFFER_SIZE 1024
#define SOCKET_ERROR -1
#define QUEUE_SIZE 5
#define ROOT_DIR argv[2] 

int main(int argc, char* argv[])
{
    int hSocket,hServerSocket;  /* handle to socket */
    struct sockaddr_in Address; /* Internet socket address stuct */
    int nAddressSize = sizeof(struct sockaddr_in);
    char pBuffer[BUFFER_SIZE];
    int nHostPort;

    if(argc < 3)
    {
	    printf("Usage: server host-port root-dir\n");
	    return 0;
    }
    else
    {
	    nHostPort = atoi(argv[1]);
    }
    /* make a socket */
    printf("Starting server at port %d...\n", nHostPort);
    hServerSocket = socket(AF_INET,SOCK_STREAM,0);
    if(hServerSocket == SOCKET_ERROR)
    {
        printf("\nCould not make a socket\n");
        return 0;
    }
    /* fill address struct */
    Address.sin_addr.s_addr = INADDR_ANY;
    Address.sin_port = htons(nHostPort);
    Address.sin_family = AF_INET;


    /* bind to a port */
    if(bind(hServerSocket, (struct sockaddr*)&Address, sizeof(Address)) == SOCKET_ERROR)
    {
        printf("\nCould not connect to host\n");
        return 0;
    }

    /* establish listen queue */
    if(listen(hServerSocket,QUEUE_SIZE) == SOCKET_ERROR)
    {
        printf("\nCould not listen\n");
        return 0;
    }

    signal(SIGCHLD, SIG_IGN);

    for(;;)
    {
        int sz_read;
        /* get the connected socket */
        hSocket=accept(hServerSocket,(struct sockaddr*)&Address,(socklen_t *)&nAddressSize);
        pBuffer[sz_read = read(hSocket,pBuffer,BUFFER_SIZE)] = EOF;
        // write(1, pBuffer, sz_read);

        if(fork() == 0)
        {
            int p2c[2];
            int c2p[2];
            pipe(p2c);
            pipe(c2p);
            if (fork() == 0)
            {
                /* close unnecessary pipe ends */
                close(p2c[1]);
                close(c2p[0]);
                /* turn stdios to pipes */
                close(0);
                dup(p2c[0]);
                close(1);
                dup(c2p[1]);
                serve(ROOT_DIR);
                /* close pipe ends */
                close(p2c[0]);
                close(c2p[1]);
                exit(0);
            }
            else
            {
                close(p2c[0]);
                close(c2p[1]);
                write(p2c[1], pBuffer, sz_read);
                while ((sz_read = read(c2p[0], pBuffer, BUFFER_SIZE)) > 0)
                    write(hSocket, pBuffer, sz_read);
            }

            close(hSocket);
            exit(0);
        }
        close(hSocket);
    }
}
