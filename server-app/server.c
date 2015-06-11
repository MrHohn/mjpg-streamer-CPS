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

/******** serverThread() *********************
 There is a separate instance of this function 
 for each connection.  It handles all communication
 once a connnection has been established.
 *****************************************/
void serverThread (int sock)
{
    int n;
    // char buffer[256];
    char buffer[BUFFER_SIZE];
    // char file_name[] = "./pics/client.jpg";
    // char file_name[BUFFER_SIZE];
    char *file_name;
    int length = 0;  
    int write_length = 0;

    // Receive the header
    char header[] = "transmit";
    char response[] = "ok";
    bzero(buffer,BUFFER_SIZE);
    n = read(sock, buffer, sizeof(buffer));
    // n = read(sock,buffer, sizeof(header));
    if (n < 0) error("ERROR reading from socket");
    printf("[server] header content: %s\n",buffer);

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

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, pid;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;

     // if (argc < 2) {
     //     fprintf(stderr,"ERROR, no port provided\n");
     //     exit(1);
     // }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     else printf ("[server] obtain socket descriptor successfully.\n"); 
     bzero((char *) &serv_addr, sizeof(serv_addr));
     // portno = atoi(argv[1]);
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
     clilen = sizeof(cli_addr);
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
         else close(newsockfd);
     } /* end of while */
     close(sockfd);
     return 0; /* we never get here */
}

