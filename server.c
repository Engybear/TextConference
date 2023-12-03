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

#include <pthread.h>

#include <assert.h>

#include "database.h"

//server should wait for connections from the clients in the system
//hardcode in a database of users
//database needs to keep track of sessionID, IP address, and client port addresses
//server will be responsible for connecting clients together in conference rooms and deleting conference rooms

struct userChecks{
    char *id;
    char *pwd;
};

struct threadArgs{
    char ip_address[1024];
    int sockfd;
};

    

const int BUFFER_SZ = 1000;
const int PACKET_SZ = 2011; // 4 + 4 + 1000 + 1000 + 3
const int MAX_USERS = 100;

struct userInfo listOfUsers[100];
struct userChecks acceptedUsers[100];
int availalbeClientNums[100] = {0}; //0 means available, 1 means taken

void *clientHandler(void *args){
    struct threadArgs *arg = (struct threadArgs *)args; 
    
    char ip_address[1024];
    for(int i = 0; i < 1024; i++) ip_address[i] = arg->ip_address[i];
    int connfd = arg->sockfd;
    printf("start of thread connfd: %d\n",connfd);

    while(1){
        printf("start of loop\n");
        char buff[PACKET_SZ];
        bzero(buff, PACKET_SZ);

        int availableNum = 0;
        for(; availableNum < MAX_USERS; availableNum++) if(availalbeClientNums[availableNum] == 0) break; //get us to an available client number in database
        
        listOfUsers[availableNum].IP = ip_address;

        read(connfd,buff,sizeof(buff));
        printf("%s\n",buff);

        char *receiveInfo;
        receiveInfo = strtok(buff,","); //get packet type
        int packetType = atoi(receiveInfo);

        receiveInfo = strtok(NULL,","); //get data length
        int dataLen = atoi(receiveInfo);

        receiveInfo = strtok(NULL,","); //get clientID
        //need to check if client id is already in database
        int clientExists = 0;
        int existingNum = 0;
        for(; existingNum < availableNum; existingNum++){ //
        
            printf("%s vs. %s\n",listOfUsers[existingNum].clientID,receiveInfo);
            if(strcmp(listOfUsers[existingNum].clientID,receiveInfo) == 0){
                clientExists = 1;
                break;
            }
        }
        if(clientExists) {//reset available num to existing num location, to use already existing client number
            availableNum = existingNum; 
            printf("client already exists!\n");
        }
        else { //if it doesn't exist, use that available num location and treat as new client
            printf("new client! %d\n",availableNum);
            availalbeClientNums[availableNum] = 1;
            
            listOfUsers[availableNum].clientID = malloc(strlen(receiveInfo) + 1);
            strcpy(listOfUsers[availableNum].clientID,receiveInfo);
            listOfUsers[availableNum].sessionID = NULL;    
            listOfUsers[availableNum].PORT = -1; 
            listOfUsers[availableNum].loggedIn = 0;
        }

        int accepted = 0;
        switch(packetType){
            case LOGIN:
                //check password correct
                printf("%s\n",acceptedUsers[0].id);
                printf("start of login\n");
                
                receiveInfo = strtok(NULL,","); //get pwd
                listOfUsers[availableNum].pwd = receiveInfo;


                printf("begin testing\n");

            
                for(int j = 0; j < 2; j++){
                    if(strcmp(acceptedUsers[j].id,listOfUsers[availableNum].clientID) == 0
                     && strcmp(acceptedUsers[j].pwd,listOfUsers[availableNum].pwd) == 0){
                        // printf("insane\n");
                        // printf("comparing %s to ",acceptedUsers[0].id);
                        // printf("%s\n",listOfUsers[availableNum].clientID);

                        // printf("comparing %s to ",acceptedUsers[j].pwd);
                        // printf("%s\n",listOfUsers[availableNum].pwd);
                        accepted = 1;
                        break;
                    }
                }
                printf("accept check\n");
                if(accepted == 1 && listOfUsers[availableNum].loggedIn == 0){
                    printf("successful login!\n");
                    //send LO_ACK
                    listOfUsers[availableNum].loggedIn = 1;

                    struct message *packet = malloc(PACKET_SZ);
                    packet->type = LO_ACK;
                    packet->size = 0;
                    packet->data[0] = 0;
                    for(int j = 0; j < BUFFER_SZ; j++) packet->source[j] = listOfUsers[availableNum].clientID[j];

                    char *packetSend = malloc(PACKET_SZ);
                    sprintf(packetSend, "%d,%d,%s,%s",packet->type, packet->size, packet->source, packet->data);
                    printf("printing packet to send: %s\n",packetSend);
    
                    write(connfd,packetSend, strlen(packetSend));

                    // free(packet);
                    // free(packetSend);  
                    
                    printf("we escape login\n");       
                }
                else {
                    printf("invalid login\n");

                    struct message *packet = malloc(PACKET_SZ);
                    packet->type = LO_NAK;

                    char *reason = "Wrong Password";
                    packet->size = strlen(reason);

                    for(int j = 0; j < strlen(reason); j++) packet->data[j] = reason[j];
                    for(int j = 0; j < BUFFER_SZ; j++) packet->source[j] = listOfUsers[availableNum].clientID[j];

                    char *packetSend = malloc(PACKET_SZ);
                    sprintf(packetSend, "%d,%d,%s,%s",packet->type, packet->size, packet->source, packet->data);
                    printf("printing packet to send: %s\n",packetSend);

                    write(connfd,packetSend, strlen(packetSend));

                    free(packet);
                    free(packetSend);     
                      
                }
            break;

            case EXIT:
                listOfUsers[availableNum].loggedIn = 0;
            break;

            case JOIN:
                //join session id
                //check if client is already in a session
                //check if session id is valid 


            break;

            case LEAVE_SESS:
            break;
            
            case NEW_SESS:
                //new sesssion
                // for()
                printf("new session wanted\n");
            break;
            
            case MESSAGE:
            break;
            
            case QUERY:
            break;

        }
    printf("we get to the end of the while loop\n");
    
    }

}

int main(int argc, char *argv[]){
    acceptedUsers[0].id = "name"; acceptedUsers[0].pwd = "pwd";
    acceptedUsers[1].id = "Bob"; acceptedUsers[1].pwd = "bob123";

    if(argc != 2){
        fprintf(stderr,"usage: server port needed\n");
		exit(1);
    }   

    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in serv_addr;
    bzero(&serv_addr,sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if((bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) != 0){
        printf("failed to bind?\n");
        exit(0);
    }
    

    listen(sockfd,MAX_USERS); //queue max users

    int i = 0;
    while(1){
        struct sockaddr_storage client;
        socklen_t cli_len = sizeof(client);
        int connfd = accept(sockfd, (struct sockaddr*)&client, &cli_len);
        if(connfd < 0) printf("connfd failed\n");
        printf("connect checks out\n");
        
        char ip_address[1024];
        inet_ntop(client.ss_family, &((struct sockaddr_in*)&client)->sin_addr, ip_address, sizeof(ip_address));
        printf("Received connection from: %s\n",ip_address);

        struct threadArgs *args = malloc(sizeof(struct threadArgs)); 
        for(int i = 0; i < 1024; i++) args->ip_address[i] = ip_address[i];
        args->sockfd = connfd;

        pthread_create(&listOfUsers[i].thread, NULL, clientHandler, args);
        i++;
    }
    
}