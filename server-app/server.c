/*************************************************
        AUTHOR: Zihong Zheng
*************************************************/

/* 
    A server in the internet domain using TCP
    The port number is set in the code
    It runs forever, forking off a separate 
    process for each connection
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
// #include <errno.h>

#define BUFFER_SIZE               1024  
#define PORT_NO                  20001
#define MAX_CONNECTION              10

/*
 * keep context for each server
 */
// context servers[MAX_SERVER_NUM];
 // context transmitServer;
 // context resultServer;


void error(const char *msg)
{
    perror(msg);
    exit(1);
}

// function for sending back the result 
void server_result (int sock)
{
    int n;
    // char buffer[BUFFER_SIZE];
    char response[] = "ok";
    char userLine[256];
    // int userNum;
    printf("result part\n");

    // reponse to the client
    n = write(sock, response, sizeof(response));
    if (n < 0) 
        error("ERROR writting to socket");

    while(1) 
    {
        // read user input and send it to client(a simple simulation)
        if (fgets(userLine, sizeof(userLine), stdin)) {
            n = write(sock, userLine, sizeof(userLine));
            if (n < 0) 
                error("ERROR writting to socket");
        }

    }

    close(sock); 
    printf("[server] Connection closed.\n\n");
    pthread_exit(NULL); //terminate calling thread!

}

// function for transmitting the frames
void server_transmit (int sock)
{
    int n;
    char buffer[BUFFER_SIZE];
    char response[] = "ok";

    char *file_name;
    int write_length = 0;
    int length = 0;  
    printf("transmitting part\n");

    // reponse to the client
    n = write(sock, response, sizeof(response));
    if (n < 0) 
        error("ERROR writting to socket");

    bzero(buffer,BUFFER_SIZE);
    n = read(sock,buffer, sizeof(buffer));
    if (n < 0) 
        error("ERROR reading from socket");
    printf("[server] file name: %s\n",buffer);
    // n = write(sock,"I got your message",18);
    // if (n < 0) error("ERROR writing to socket");

    file_name = buffer;
    FILE *fp = fopen(buffer, "w");  
    if (fp == NULL)  
    {  
        printf("File:\t%s Can Not Open To Write!\n", file_name);  
        exit(1);  
    }  

    // receive the data from server and store them into buffer
    bzero(buffer, sizeof(buffer));  
    while((length = recv(sock, buffer, BUFFER_SIZE, 0)))  
    {  
        if (length < 0)  
        {  
            printf("Recieve Data From Client Failed!\n");  
            break;  
        }  
  
        write_length = fwrite(buffer, sizeof(char), length, fp);  
        if (write_length < length)  
        {  
            printf("File:\t Write Failed!\n");  
            break;  
        }  
        bzero(buffer, BUFFER_SIZE);  
    }  
  
    printf("[server] Recieve File:\t%s From Client Finished!\n", file_name);  
  
    // finished 
    fclose(fp);
    

    close(sock); 
    printf("[server] Connection closed.\n\n");
    pthread_exit(NULL); //terminate calling thread!

}

/******** serverThread() *********************
 There is a separate instance of this function 
 for each connection.  It handles all communication
 once a connnection has been established.
 *****************************************/
void *serverThread (void * inputsock)
{
    int sock = *((int *)inputsock);
    int n;
    char buffer[20];

    // Receive the header
    bzero(buffer,20);
    n = read(sock, buffer, sizeof(buffer));
    if (n < 0) error("ERROR reading from socket");
    printf("[server] header content: %s\n",buffer);

    if (strcmp(buffer, "transmit") == 0) 
    {
        server_transmit(sock);
    }
    else if (strcmp(buffer, "result") == 0) 
    {
        server_result(sock);
    }
    else
    {
        close(sock); 
        printf("[server] Command Unknown. Connection closed.\n\n");
    }

    return 0;
}

void run_server()
{
    // init part
    // int pid;
    printf("\n[server] start initializing\n");
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    clilen = sizeof(cli_addr);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("ERROR opening socket\n");
        error("ERROR opening socket");
    } 
    else 
        printf ("[server] obtain socket descriptor successfully.\n"); 
    bzero((char *) &serv_addr, sizeof(serv_addr));
    // set up the port number
    portno = PORT_NO;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    {
        printf("ERROR on binding");
        error("ERROR on binding");
    }
    else
        printf("[server] bind tcp port %d sucessfully.\n",portno);

    if(listen(sockfd,5))
        error("ERROR listening");
    else 
        printf ("[server] listening the port %d sucessfully.\n", portno);    
    
    // init finished, now wait for a client
    while (1) {
        pthread_t thread_id;
        //Block here. Until server accpets a new connection.
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)
        {
            // error("ERROR on accept");
            fprintf(stderr,"Accept error!\n");
            continue; //ignore current socket ,continue while loop.
        }
        else 
            printf ("\n[server] server has got connect from %s.\n", (char *)inet_ntoa(cli_addr.sin_addr));

        // pid = fork();
        // if (pid < 0)
        //     error("ERROR on fork");
        // if (pid == 0)  {
        //     close(sockfd);
        //     serverThread(newsockfd);
        //     exit(0);
        // }
        // else close(newsockfd);

        /* create thread and pass context to thread function */
        if (pthread_create(&thread_id, 0, (void *)serverThread, (void *)&(newsockfd)) == -1)
        {
            fprintf(stderr,"pthread_create error!\n");
            break; //break while loop
        }
        pthread_detach(thread_id);

    } /* end of while */
    close(sockfd);
}

int main(int argc, char *argv[])
{
    run_server();
    return 0; /* we never get here */
}

