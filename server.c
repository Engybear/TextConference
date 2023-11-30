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
    char buff[1024];
    const int MAX_USERS = 100;

    for(int i = 0; i < MAX_USERS; i++){

        listen(sockfd,MAX_USERS);

        struct sockaddr_storage client;
        socklen_t cli_len = sizeof(client);
        int connfd = accept(sockfd, (struct sockaddr*)&client, &cli_len);
        if(connfd < 0) printf("connfd failed\n");

        char str[1024];
        inet_ntop(client.ss_family, &((struct sockaddr_in*)&client)->sin_addr, str, sizeof(str));
        printf("Received connection from: %s\n",str);

        //wait constantly for client ids to setup
        //on setup, what do we do?


        read(connfd,buff,sizeof(buff));
        printf("%s\n",buff);

        

        listOfUsers[i].clientID;
        listOfUsers[i].pwd;
        listOfUsers[i].sessionID = -1;

        //check against hardcoded database
            //send LO_ACK or LO_NAK

    
    }

    // char buff[100];
    // bzero(buff,100);
    // read(connfd, buff, sizeof(buff));
    // printf("client said: %s",buff);

    // char* buff2 = "hello from server";
    // write(connfd, buff2, strlen(buff2));


    
}