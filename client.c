#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>


#define SOCK_PORT 4040
#define THREAD_LIMIT 100

int createConnection()
{
    int iClientFd = socket(AF_INET,SOCK_STREAM,0);
    if(iClientFd < 0){
        perror("Socket couldn't be created!\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(SOCK_PORT);
    socklen_t addrlen = sizeof(address);
    if((inet_pton(AF_INET,"127.0.0.1",&address.sin_addr)) <= 0){
        perror("inet_pton error!\n");
        exit(EXIT_FAILURE);
    }

    int connectionStatus = connect(iClientFd,(struct sockaddr*)&address,addrlen);
    if(connectionStatus < 0){
        perror("Connection error!\n");
        exit(EXIT_FAILURE);
    }

    return iClientFd;
}

void* runClient(void *args){
    int iClientFd = createConnection();
    char szSendMessage[100] = {0};
    char szRecvMessage[1025] = {0};
    int iRecvSize;
    sprintf(&szSendMessage[0],"Hello, this is message from client %d\n",iClientFd);
    send(iClientFd,&szSendMessage,strlen(szSendMessage),0);
    while(1){
        iRecvSize = recv(iClientFd,&szRecvMessage,1024,0);
        if(iRecvSize == 0){
            break;
        }
        printf("Reply from server : %s",szRecvMessage);
    }
    close(iClientFd);
    return NULL;
}

int main()
{

    pthread_t tempTask;
    for(int i = 0;i<THREAD_LIMIT;i++){
        if((pthread_create(&tempTask,NULL,&runClient,NULL)) < 0){
            perror("Thread couldn't be created!\n");
            exit(EXIT_FAILURE);
        }
    }
    return 0;   
}