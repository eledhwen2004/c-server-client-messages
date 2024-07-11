#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <memory.h>

#define SOCK_PORT 4040
#define ACTIVE_SOCKET_LIMIT 10

struct timespec remaining1, request1 = { 0, 1 * 1000000 };

int iaSocketsFd[ACTIVE_SOCKET_LIMIT] = {0};
int iActiveSockNum = 0;
int iEmptySockIndex = 0;
//int iRequestNum = 0;

pthread_mutex_t lock;

void *handleConnectionTask(void *args)
{
    int iNewSocketFd = (int*)args;
    pthread_mutex_lock(&lock);
    iActiveSockNum++;
    //iRequestNum++;

    char szRecvMessage[1025] = {0};
    char szBuffer[1025] = {0};
    int iRecvSize;
    int iBufferIndex = 0;
    int iRecvIndex = 0;
    while(1){
        iRecvSize = recv(iNewSocketFd,&szRecvMessage,1024,0);
        if(iRecvSize == 0){
            printf("iRecvSize = 0\n");
            break;
        }
        while(iRecvIndex != iRecvSize){
            while(szRecvMessage[iRecvIndex] != '-'){
                if(iRecvIndex == iRecvSize){
                    break;
                }
                szBuffer[iBufferIndex] = szRecvMessage[iRecvIndex];
                iRecvIndex++;
                iBufferIndex++;
            }
            if(iRecvIndex == iRecvSize){
                break;
            }
            szBuffer[iBufferIndex] = 0;
            iRecvIndex++;
            printf("Message from client %d : %s",iNewSocketFd,szBuffer);
            iBufferIndex = 0;
            memset(szBuffer,0,1025);
        }
        iRecvIndex = 0;
    }
    
    for(int i = 0;i<ACTIVE_SOCKET_LIMIT;i++){
        if(iNewSocketFd == iaSocketsFd[i]){
            iaSocketsFd[i] = 0;
            break;
        }
    }

    shutdown(iNewSocketFd, SHUT_RDWR);
    close(iNewSocketFd);
    iActiveSockNum--;
    pthread_mutex_unlock(&lock);
    printf("Socket closed!\n");
    return NULL;
}

int main()
{
    int iServerFd = socket(AF_INET, SOCK_STREAM, 0);
    if(iServerFd < 0){
        perror("Socket couldn't be created!\n");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(SOCK_PORT); 
    address.sin_addr.s_addr = INADDR_ANY;
    socklen_t addrlen = sizeof(address);

    int iSockOpt = 1;
    if(setsockopt(iServerFd,SOL_SOCKET,SO_REUSEADDR,&iSockOpt,sizeof(iSockOpt)) < 0){
        perror("setsockopt failed!\n");
        exit(EXIT_FAILURE);
    }

    if((bind(iServerFd,(struct sockaddr*)&address,addrlen)) < 0){
        perror("Socket couldn't be binded!\n");
        exit(EXIT_FAILURE);
    }

    if(listen(iServerFd,100) < 0){
        perror("Socket couldn't be listened!\n");
        exit(EXIT_FAILURE);
    }

    int iNewSocketFd;
    pthread_t tempTask;
    while(1){
        while(1){
            if(iActiveSockNum < 10){
                break;
            }
            printf("Connection waiting!\n");
            nanosleep(&request1, &remaining1);
        }
        for(int i = 0;i< ACTIVE_SOCKET_LIMIT;i++){
            if(iaSocketsFd[i] == 0){
                iEmptySockIndex = i;
                break;
            }
        }
        iaSocketsFd[iEmptySockIndex] = accept(iServerFd,(struct sockaddr*)&address,&addrlen);
        if(iaSocketsFd[iEmptySockIndex] < 0){
            perror("Connection couldn't be accepted!\n");
            exit(EXIT_FAILURE);
        }
        if((pthread_create(&tempTask,NULL,handleConnectionTask,(void *)iaSocketsFd[iEmptySockIndex]) )!= 0){
            perror("Thread couldn't be created!\n");
            exit(EXIT_FAILURE);
        }
    }


    shutdown(iServerFd, SHUT_RDWR);
    close(iServerFd);
    return 0;
}
