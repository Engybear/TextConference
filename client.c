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

const int BUFFER_SZ = 1000;
const int PACKET_SZ = 2011; // 4 + 4 + 1000 + 1000

char inputBuf[1000];

struct clientInfo{
    char *clientID; //1000
    char *sessionID; //1000
    int sockfd; //4
    struct sockaddr_in serv_addr;
    
};

struct clientInfo *client;

void login(char *inputSlice);
void logout();
void joinSess(char *inputSlice);
void leaveSess();
void createSess(char *inputSlice);
void list();
void quit();
void text();

const int clientInfoSZ = 2004;

int main(){

    char *inputSlice;
    
    client = malloc(clientInfoSZ + sizeof(client->serv_addr));
    // client->clientID = malloc(1000);
    // client->sessionID = malloc(1000);
    client->sockfd = -1;
    bzero(&client->serv_addr, sizeof(client->serv_addr));


    while(1){
        // scanf("%[^\n]s",inputBuf);
        bzero(inputBuf, BUFFER_SZ - 1);
        fgets(inputBuf, BUFFER_SZ - 1, stdin);
        inputBuf[strlen(inputBuf) - 1] = '\0'; //set end of inputBuf to null instead of \n    
        inputSlice = inputBuf;
        if(*inputSlice == '\0') continue;
        
        inputSlice = strtok(inputBuf," ");

        if(strcmp(inputSlice, "/login") == 0){
            printf("login cmd\n");
            login(inputSlice);

        }else if(strcmp(inputSlice,"/logout") == 0){
            printf("logout\n");
            if(client->sockfd != -1) client->sockfd = -1;
            
        }else if(strcmp(inputSlice,"/joinsession") == 0){
            printf("join\n");
            joinSess(inputSlice);
            
        }else if(strcmp(inputSlice,"/leavesession") == 0){
            printf("leave\n");
            leaveSess();
            
        }else if(strcmp(inputSlice,"/createsession") == 0){
            printf("create\n");
            createSess(inputSlice);
            
        }else if(strcmp(inputSlice,"/list") == 0){
            printf("list\n");
            list();
            
        }else if(strcmp(inputSlice,"/quit") == 0){
            printf("quit\n");
            quit();

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


void login(char *inputSlice){
    
    if(client->sockfd != -1){
        printf("Already connected to a server\n");
        return; //already connected
    } 
    client->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(client->sockfd == -1) {
        printf("socket creation failed\n");
        exit(0);
    }
    
    //put client into database for the server

    char *ID, *pwd, *serverIP, *serverPort;
    inputSlice = strtok(NULL, " ");
    ID = strdup(inputSlice);
    // save id into global variable
    client->clientID = ID;
    inputSlice = strtok(NULL, " ");
    pwd = inputSlice;
    inputSlice = strtok(NULL, " ");
    serverIP = inputSlice;
    inputSlice = strtok(NULL, "\0");
    serverPort = inputSlice;

    client->serv_addr.sin_family = AF_INET;
    client->serv_addr.sin_addr.s_addr = inet_addr(serverIP); //take input from user
    client->serv_addr.sin_port = htons(atoi(serverPort)); //take input from user

    if(connect(client->sockfd, (struct sockaddr*)&client->serv_addr, sizeof(client->serv_addr)) != 0){
        printf("unable to connect with server\n");
        client->sockfd = -1;
        return;
    }
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

    printf("Packet to send: %s\n",packetSend);
    
    write(client->sockfd,packetSend, strlen(packetSend));

    char buff[BUFFER_SZ];
    bzero(buff, sizeof(buff));

    read(client->sockfd, buff, sizeof(buff));
    inputSlice = strtok(buff, ",");
    if(atoi(inputSlice) == LO_NAK){
        client->sockfd = -1;
        printf("Login was unsuccessful\n");
    }
    else{
        printf("Login was successful\n");
    }
    return;
    
}

void joinSess(char *inputSlice){
    if(client->sockfd == -1){
        printf("Not connected to a server\n");
        return; //already connected
    } 
    char buff[BUFFER_SZ];
    bzero(buff, sizeof(buff));
    // extract the session ID from the user (doesnt have to be a number)
    char *sessionID;
    struct message *packet = malloc(PACKET_SZ);
    inputSlice = strtok(NULL, " ");
    sessionID = inputSlice;
    client->sessionID = sessionID;
    // format the message
    packet->type = JOIN;
    packet->size = strlen(sessionID);
    for(int i = 0; i < strlen(client->clientID); i++) packet->source[i] = client->clientID[i];
    for(int i = 0; i < strlen(client->sessionID); i++) packet->data[i] = sessionID[i];

    // send message as one contiguous string
    char *packetSend = malloc(PACKET_SZ);
    sprintf(packetSend, "%d,%d,%s,%s",packet->type, packet->size, packet->source, packet->data);
    printf("printing packet to send: %s\n",packetSend);

    // send request to join session to server
    write(client->sockfd,packetSend, strlen(packetSend));

    // printf("Creating new session...\n");

    // wait for ACK / NACK
    read(client->sockfd, buff, sizeof(buff));
    inputSlice = strtok(buff, ",");
    if(atoi(inputSlice) == JN_NAK){
        printf("Joining new session was unsuccessful\n");
    }
    else if(atoi(inputSlice) == JN_ACK){
        printf("Joining new session was successful\n");
    }
    return;
}

void createSess(char *inputSlice){
    if(client->sockfd == -1){
        printf("Not connected to a server\n");
        return; //already connected
    } 
    char buff[BUFFER_SZ];
    bzero(buff, sizeof(buff));
    // extract the session ID from the user (doesnt have to be a number)
    char *sessionID = malloc(1000);
    struct message *packet = malloc(PACKET_SZ);
    inputSlice = strtok(NULL, " ");
    sessionID = inputSlice;
    client->sessionID = sessionID;
    // format the message
    packet->type = NEW_SESS;
    packet->size = strlen(sessionID);
    for(int i = 0; i < strlen(client->clientID); i++) packet->source[i] = client->clientID[i];
    for(int i = 0; i < strlen(client->sessionID); i++) packet->data[i] = sessionID[i];

    // send message as one contiguous string
    char *packetSend = malloc(PACKET_SZ);
    sprintf(packetSend, "%d,%d,%s,%s",packet->type, packet->size, packet->source, packet->data);
    printf("printing packet to send: %s\n",packetSend);
    
    // send request to create the session to the server
    write(client->sockfd,packetSend, strlen(packetSend));
    
    // wait for ACK/NACK
    read(client->sockfd, buff, sizeof(buff));

    inputSlice = strtok(buff, ",");
    if(atoi(inputSlice) != NS_ACK){
        printf("Creating new session was unsuccessful\n");
    }
    else{
        printf("Creating new session was successful\n");
    }
    

    return;
}

void leaveSess(){
    if(client->sockfd == -1){
        printf("Not connected to a server\n");
        return; //already connected
    } 
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
    write(client->sockfd,packetSend, strlen(packetSend));
    printf("Left session successfully\n");

    return;
}

void list(){
    if(client->sockfd == -1){
        printf("Not connected to a server\n");
        return; //already connected
    } 
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
    write(client->sockfd,packetSend, strlen(packetSend));

    // wait for ACK/NACK
    read(client->sockfd, buff, sizeof(buff));
    printf("List of clients and sessions:\n%s", buff);

    return;
}

void quit(){
    if(client->sockfd == -1){
        printf("Not connected to a server\n");
        return; //already connected
    } 
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
    write(client->sockfd,packetSend, strlen(packetSend));

    // wait for ack?
    read(client->sockfd, buff, sizeof(buff));
    // parse the buffer message from the server
    return;
}

void logout(){
    if(client->sockfd == -1){
        printf("Not connected to a server\n");
        return; //already connected
    } 
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
    write(client->sockfd,packetSend, strlen(packetSend));

    // wait for ack?
    read(client->sockfd, buff, sizeof(buff));
    // parse the buffer message from the server
    return;
}