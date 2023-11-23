#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <assert.h>

#include "database.h"

// 8 user input commands for client to implement
/*
1. /login clientID, pwd, serverIP, serverPort - log into server
2. /logout - exit server
3. /joinsession sessionID - join a specific session
4. /leaveession - leave the session
5. /createsession sessionID - create a new conference session and join it
6. /list - get list of connected clients and available sessions
7. /quit - terminate the program
8. <text> - send text while in a session 
*/
void login(char *inputSlice);
void logout();
void joinSess();
void leaveSess();
void list();
void text();

const int BUFFER_SZ = 1000;

int main(){


    char inputBuf[BUFFER_SZ];
    char *inputSlice;
    while(1){
        // scanf("%[^\n]s",inputBuf);
        fgets(inputBuf, BUFFER_SZ - 1, stdin);
        inputBuf[strlen(inputBuf) - 1] = '\0'; //set end of inputBuf to null instead of \n    
        
        inputSlice = inputBuf;
        if(*inputSlice == '\0') continue;
        
        inputSlice = strtok(inputBuf," ");

        if(strcmp(inputSlice, "/login") == 0){
            printf("login cmd\n");

            //if login, check if already logged in
            int sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if(sockfd == -1) {
                printf("socket creation failed\n");
                exit(0);
            }
            struct sockaddr_in serv_addr;
            bzero(&serv_addr, sizeof(serv_addr));

            //put client into database for the server

            char *clientID, *pwd, *serverIP, *serverPort;
            inputSlice = strtok(NULL, " ");
            clientID = inputSlice;
            inputSlice = strtok(NULL, " ");
            pwd = inputSlice;
            inputSlice = strtok(NULL, " ");
            serverIP = inputSlice;
            inputSlice = strtok(NULL, "\0");
            serverPort = inputSlice;

            printf("recombining split msg: %s | %s | %s | %s\n",clientID,pwd,serverIP,serverPort);

            serv_addr.sin_family = AF_INET;
            serv_addr.sin_addr.s_addr = inet_addr(serverIP); //take input from user
            serv_addr.sin_port = htons(atoi(serverPort)); //take input from user

            if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != 0){
                printf("unable to connect with server\n");
                // exit(0);
            }

            //write clientID and pwd to server

            printf("started packet making\n");

            struct message packet;
            packet.type = LOGIN;
            packet.size = sizeof(packet);
            // packet.source = clientID;

            //convert data into packetc
            char *data = malloc(1000);
            sprintf(data,"%s %s %s %s",clientID,pwd,serverIP,serverPort);
            
            for(int j = 0; j < 1000; j++){
                // printf("%c",data[j]);
                packet.data[j] = data[j];
            } 
            printf("packet data: %s",packet.data);

            char *packetSend = malloc(1000);
            sprintf(packetSend, "%d,%d,%s,%s",packet.type, packet.size, packet.source, packet.data);
            // char* buff = "hello from client";
            write(sockfd,packetSend, strlen(packetSend));
            
            // char buff2[100];
            // read(sockfd,buff2,sizeof(buff2));
            // printf("server said: %s\n",buff2);



        }else if(strcmp(inputSlice,"/logout") == 0){
            printf("logout\n");
            
        }else if(strcmp(inputSlice,"/joinsession") == 0){
            printf("join\n");
            
        }else if(strcmp(inputSlice,"/leavesession") == 0){
            printf("leave\n");
            
        }else if(strcmp(inputSlice,"/createsession") == 0){
            printf("create\n");
            
        }else if(strcmp(inputSlice,"/list") == 0){
            printf("list\n");
            
        }else if(strcmp(inputSlice,"/quit") == 0){
            printf("quit\n");

            break;
            
        }else{ //just text to send
        
            inputBuf[strlen(inputSlice)] = ' '; //restores buffer
            printf("inputBuf restored: %s\n",inputBuf);
            
            //check if in server and in a session to send text
            printf("Invalid command or text to send\n");
        }
    }
    printf("program quiting\n");
    exit(0);
}