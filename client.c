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
1. /login ID, pwd, serverIP, serverPort - log into server
2. /logout - exit server
3. /joinsession sessionID - join a specific session
4. /leaveession - leave the session
5. /createsession sessionID - create a new conference session and join it
6. /list - get list of connected clients and available sessions
7. /quit - terminate the program
8. <text> - send text while in a session 
*/

const int BUFFER_SZ = 1024;
const int PACKET_SZ = 2008; // 4 + 4 + 1000 + 1000

char inputBuf[1000];

struct clientInfo{
    char *clientID;
    char *sessionID;
    int sockfd;
};

struct clientInfo *client;

void login(int sockfd, char *inputSlice);
void logout(int sockfd);
void joinSess(int sockfd, char *inputSlice);
void leaveSess(int sockfd);
void createSess(int sockfd, char *inputSlice);
void list(int sockfd);
void quit(int sockfd);
void text();



int main(){

    char *inputSlice;
    int sockfd = -1;

    while(1){
        // scanf("%[^\n]s",inputBuf);
        fgets(inputBuf, BUFFER_SZ - 1, stdin);
        inputBuf[strlen(inputBuf) - 1] = '\0'; //set end of inputBuf to null instead of \n    
        client = malloc(sizeof(struct clientInfo));
        inputSlice = inputBuf;
        if(*inputSlice == '\0') continue;
        
        inputSlice = strtok(inputBuf," ");

        if(strcmp(inputSlice, "/login") == 0){
            printf("login cmd\n");
            login(sockfd, inputSlice);

        }else if(strcmp(inputSlice,"/logout") == 0){
            printf("logout\n");
            if(sockfd != -1) sockfd = -1;
            
        }else if(strcmp(inputSlice,"/joinsession") == 0){
            printf("join\n");
            joinSess(sockfd, inputSlice);
            
        }else if(strcmp(inputSlice,"/leavesession") == 0){
            printf("leave\n");
            leaveSess(sockfd);
            
        }else if(strcmp(inputSlice,"/createsession") == 0){
            printf("create\n");
            createSess(sockfd, inputSlice);
            
        }else if(strcmp(inputSlice,"/list") == 0){
            printf("list\n");
            list(sockfd);
            
        }else if(strcmp(inputSlice,"/quit") == 0){
            printf("quit\n");
            quit(sockfd);

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


void login(int sockfd, char *inputSlice){
    
    if(sockfd != -1) return; //already connected
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        printf("socket creation failed\n");
        exit(0);
    }
    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));

    //put client into database for the server

    char *ID, *pwd, *serverIP, *serverPort;
    inputSlice = strtok(NULL, " ");
    ID = inputSlice;
    // save id into global variable
    client->clientID = ID;
    inputSlice = strtok(NULL, " ");
    pwd = inputSlice;
    inputSlice = strtok(NULL, " ");
    serverIP = inputSlice;
    inputSlice = strtok(NULL, "\0");
    serverPort = inputSlice;

    printf("split msg: %s | %s | %s | %s\n",ID,pwd,serverIP,serverPort);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(serverIP); //take input from user
    serv_addr.sin_port = htons(atoi(serverPort)); //take input from user

    if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != 0){
        printf("unable to connect with server\n");
        return;
    }

    printf("started packet making\n");
    struct message *packet = malloc(PACKET_SZ);
    packet->type = LOGIN; //assign packet type

    for(int i = 0; i < strlen(ID); i++) packet->source[i] = ID[i]; //assign packet source

    //assign packet data
    char *data = malloc(1000);
    data = pwd;
    for(int j = 0; j < 1000; j++) packet->data[j] = data[j];

    packet->size = strlen(data); //assign packet data length
    
    //compile packet into a buffer to send
    char *packetSend = malloc(PACKET_SZ);
    sprintf(packetSend, "%d,%d,%s,%s",packet->type, packet->size, packet->source, packet->data);

    printf("printing packet to send: %s\n",packetSend);
    
    write(sockfd,packetSend, strlen(packetSend));

    char buff[BUFFER_SZ];
    bzero(buff, sizeof(buff));

    read(sockfd, buff, sizeof(buff));
    // parse the buffer message from the server
}

void joinSess(int sockfd, char *inputSlice){
    char buff[BUFFER_SZ];
    bzero(buff, sizeof(buff));
    // extract the session ID from the user (doesnt have to be a number)
    char *sessionID;
    struct message *packet = malloc(PACKET_SZ);
    inputSlice = strtok(NULL, " ");
    sessionID = inputSlice;
    client->sessionID = sessionID;
    // format the message
    packet->type = JN_ACK;
    packet->size = strlen(sessionID);
    for(int i = 0; i < strlen(client->clientID); i++) packet->source[i] = client->clientID[i];
    for(int i = 0; i < strlen(sessionID); i++) packet->data[i] = sessionID[i];

    // send message as one contiguous string
    char *packetSend = malloc(PACKET_SZ);
    sprintf(packetSend, "%d,%d,%s,%s",packet->type, packet->size, packet->source, packet->data);
    printf("printing packet to send: %s\n",packetSend);

    // send request to join session to server
    write(sockfd,packetSend, strlen(packetSend));

    // wait for ACK / NACK
    read(sockfd, buff, sizeof(buff));
    return;
}

void createSess(int sockfd, char *inputSlice){
    char buff[BUFFER_SZ];
    bzero(buff, sizeof(buff));
    // extract the session ID from the user (doesnt have to be a number)
    char *sessionID;
    struct message *packet = malloc(PACKET_SZ);
    inputSlice = strtok(NULL, " ");
    sessionID = inputSlice;
    client->sessionID = sessionID;
    // format the message
    packet->type = NEW_SESS;
    packet->size = strlen(sessionID);
    for(int i = 0; i < strlen(client->clientID); i++) packet->source[i] = client->clientID[i];
    for(int i = 0; i < strlen(sessionID); i++) packet->data[i] = sessionID[i];

    // send message as one contiguous string
    char *packetSend = malloc(PACKET_SZ);
    sprintf(packetSend, "%d,%d,%s,%s",packet->type, packet->size, packet->source, packet->data);
    printf("printing packet to send: %s\n",packetSend);
    
    // send request to create the session to the server
    write(sockfd,packetSend, strlen(packetSend));

    // wait for ACK/NACK
    read(sockfd, buff, sizeof(buff));
    // parse the buffer message from the server

    return;
}

void leaveSess(int sockfd){
    char buff[BUFFER_SZ];
    bzero(buff, sizeof(buff));
    struct message *packet = malloc(PACKET_SZ);
    // format the message
    packet->type = LEAVE_SESS;
    packet->size = 0;
    for(int i = 0; i < strlen(client->clientID); i++) packet->source[i] = client->clientID[i];
    packet->data[0] = '\0';
    // send message as one contiguous string
    char *packetSend = malloc(PACKET_SZ);
    sprintf(packetSend, "%d,%d,%s,%s",packet->type, packet->size, packet->source, packet->data);
    printf("printing packet to send: %s\n",packetSend);

    // send request to create the session to the server
    write(sockfd,packetSend, strlen(packetSend));

    // wait for ACK/NACK
    read(sockfd, buff, sizeof(buff));
    // parse the buffer message from the server

    return;
}

void list(int sockfd){
    char buff[BUFFER_SZ];
    bzero(buff, sizeof(buff));
    struct message *packet = malloc(PACKET_SZ);
    // format the message
    packet->type = QUERY;
    packet->size = 0;
    for(int i = 0; i < strlen(client->clientID); i++) packet->source[i] = client->clientID[i];
    packet->data[0] = '\0';
    // send message as one contiguous string
    char *packetSend = malloc(PACKET_SZ);
    sprintf(packetSend, "%d,%d,%s,%s",packet->type, packet->size, packet->source, packet->data);
    printf("printing packet to send: %s\n",packetSend);

    // send request to create the session to the server
    write(sockfd,packetSend, strlen(packetSend));

    // wait for ACK/NACK
    read(sockfd, buff, sizeof(buff));
    // parse the buffer message from the server

    return;
}

void quit(int sockfd){
    char buff[BUFFER_SZ];
    bzero(buff, sizeof(buff));
    struct message *packet = malloc(PACKET_SZ);
    // format the message
    packet->type = EXIT;
    packet->size = 0;
    for(int i = 0; i < strlen(client->clientID); i++) packet->source[i] = client->clientID[i];
    packet->data[0] = '\0';
    // send message as one contiguous string
    char *packetSend = malloc(PACKET_SZ);
    sprintf(packetSend, "%d,%d,%s,%s",packet->type, packet->size, packet->source, packet->data);
    printf("printing packet to send: %s\n",packetSend);

    // send quit request
    write(sockfd,packetSend, strlen(packetSend));

    // wait for ack?
    read(sockfd, buff, sizeof(buff));
    // parse the buffer message from the server
    return;
}

void logout(int sockfd){
    char buff[BUFFER_SZ];
    bzero(buff, sizeof(buff));
    struct message *packet = malloc(PACKET_SZ);
    // format the message
    packet->type = EXIT;
    packet->size = 0;
    for(int i = 0; i < strlen(client->clientID); i++) packet->source[i] = client->clientID[i];
    packet->data[0] = '\0';
    // send message as one contiguous string
    char *packetSend = malloc(PACKET_SZ);
    sprintf(packetSend, "%d,%d,%s,%s",packet->type, packet->size, packet->source, packet->data);
    printf("printing packet to send: %s\n",packetSend);

    // send quit request
    write(sockfd,packetSend, strlen(packetSend));

    // wait for ack?
    read(sockfd, buff, sizeof(buff));
    // parse the buffer message from the server
    return;
}