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

//server should wait for connections from the clients in the system
//hardcode in a database of users
//database needs to keep track of sessionID, IP address, and client port addresses
//server will be responsible for connecting clients together in conference rooms and deleting conference rooms

struct userChecks{
    char *id;
    char *pwd;
};


const int BUFFER_SZ = 1000;
const int PACKET_SZ = 2011; // 4 + 4 + 1000 + 1000 + 3
const int MAX_USERS = 100;


int main(int argc, char *argv[]){
    struct userChecks acceptedUsers[MAX_USERS];
    acceptedUsers[0].id = "name"; acceptedUsers[0].pwd = "name pwd 127.0.0.1 1234";
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
    
    struct userInfo listOfUsers[100];
    char buff[PACKET_SZ];

    for(int i = 0; i < MAX_USERS; i++){
        printf("dun dunn dunnnnnn\n");

        int check = listen(sockfd,MAX_USERS); //queue max users
        if(check == 0) printf("successful listen\n");

        struct sockaddr_storage client;
        socklen_t cli_len = sizeof(client);
        int connfd = accept(sockfd, (struct sockaddr*)&client, &cli_len);
        if(connfd < 0) printf("connfd failed\n");

        char str[1024];
        inet_ntop(client.ss_family, &((struct sockaddr_in*)&client)->sin_addr, str, sizeof(str));
        printf("Received connection from: %s\n",str);

        listOfUsers[i].IP = str;

        read(connfd,buff,sizeof(buff));
        printf("%s\n",buff);

        char *receiveInfo;
        receiveInfo = strtok(buff,","); //get packet type
        int packetType = atoi(receiveInfo);

        receiveInfo = strtok(NULL,","); //get data length
        int dataLen = atoi(receiveInfo);

        receiveInfo = strtok(NULL,","); //get clientID
        listOfUsers[i].clientID = receiveInfo;

        if(dataLen != 0){
            receiveInfo = strtok(NULL,","); //get data
            listOfUsers[i].pwd = receiveInfo;
        }
        
        listOfUsers[i].sessionID = NULL;
        listOfUsers[i].PORT = -1;

        int accepted = 0;
        switch(packetType){
            case LOGIN:
                //check password correct
                
                for(int j = 0; j < MAX_USERS; j++){
                    if(strcmp(acceptedUsers[j].id,listOfUsers[i].clientID) == 0 && strcmp(acceptedUsers[j].pwd,listOfUsers[i].pwd) == 0){
                        accepted = 1;
                        break;
                    }
                }
                if(accepted == 1){
                    printf("successful login!\n");
                    //send LO_ACK

                    struct message *packet = malloc(PACKET_SZ);
                    packet->type = LO_ACK;
                    packet->size = 0;
                    packet->data[0] = 0;
                    printf("help?\n");
                    for(int j = 0; j < BUFFER_SZ; j++) packet->source[j] = listOfUsers[i].clientID[j];

                    printf("checkpoint\n");
                    // send message as one contiguous string
                    char *packetSend = malloc(PACKET_SZ);
                    sprintf(packetSend, "%d,%d,%s,%s",packet->type, packet->size, packet->source, packet->data);
                    printf("printing packet to send: %s\n",packetSend);

                    // send request to join session to server
                    write(connfd,packetSend, strlen(packetSend));

                    // write(); 
                    free(packet);
                    free(packetSend);        
                }
                else {
                    printf("get out of here\n");
                }
            break;

            case EXIT:
            break;

            case JOIN:
            break;

            case LEAVE_SESS:
            break;
            
            case NEW_SESS:
            break;
            
            case MESSAGE:
            break;
            
            case QUERY:
            break;

        }
        //check against hardcoded database
            //send LO_ACK or LO_NAK

    
    }


    
}