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
#define BUFFER_SIZE               1024  
#define PORT_NO                  20001

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

// function for sending back the result 
void server_result (int sock)
{
    int n;
    char buffer[BUFFER_SIZE];
    char response[] = "ok";
    char userLine[256];
    int userNum;
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

}

// function for transmitting the frames
void server_transmitFrames (int sock)
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
    if (n < 0) error("ERROR writting to socket");

    bzero(buffer,BUFFER_SIZE);
    n = read(sock,buffer, sizeof(buffer));
    if (n < 0) error("ERROR reading from socket");
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
    while(length = recv(sock, buffer, BUFFER_SIZE, 0))  
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

}

/******** serverThread() *********************
 There is a separate instance of this function 
 for each connection.  It handles all communication
 once a connnection has been established.
 *****************************************/
void serverThread (int sock)
{
    int n;
    char buffer[20];

    // Receive the header
    bzero(buffer,20);
    n = read(sock, buffer, sizeof(buffer));
    if (n < 0) error("ERROR reading from socket");
    printf("[server] header content: %s\n",buffer);

    if (strcmp(buffer, "transmit") == 0) 
    {
        server_transmitFrames(sock);
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

}

void run_server()
{
    // init part
    int sockfd, newsockfd, portno, pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    clilen = sizeof(cli_addr);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
       error("ERROR opening socket");
    else printf ("[server] obtain socket descriptor successfully.\n"); 
    bzero((char *) &serv_addr, sizeof(serv_addr));
    // set up the port number
    portno = PORT_NO;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0) 
             error("ERROR on binding");
    else printf("[server] bind tcp port %d sucessfully.\n",portno);
    if(listen(sockfd,5))
     error("ERROR listening");
    else printf ("[server] listening the port %d sucessfully.\n", portno);    
    
    // init finished, now wait for a client
    while (1) {
        newsockfd = accept(sockfd, 
              (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) 
            error("ERROR on accept");
        else printf ("\n[server] server has got connect from %s.\n", (char *)inet_ntoa(cli_addr.sin_addr));
        pid = fork();
        if (pid < 0)
            error("ERROR on fork");
        if (pid == 0)  {
            close(sockfd);
            serverThread(newsockfd);
            exit(0);
        }
        // else close(newsockfd);
    } /* end of while */
    close(sockfd);
}

int main(int argc, char *argv[])
{
    run_server();
    return 0; /* we never get here */
}

