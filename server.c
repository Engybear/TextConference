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


const int BUFFER_SZ = 1000;
const int PACKET_SZ = 2008; // 4 + 4 + 1000 + 1000

int main(int argc, char *argv[]){
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
    const int MAX_USERS = 100;

    for(int i = 0; i < MAX_USERS; i++){

        listen(sockfd,MAX_USERS); //queue max users

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

        switch(packetType){
            case LOGIN:
                
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