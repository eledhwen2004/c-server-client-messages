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
#define SOCK_HEADER_SIZE 4

struct timespec remaining1, request1 = {0, 1 * 1000000};
struct timespec remaining2, request2 = {0, 1 * 100000};

int iaSocketsFd[ACTIVE_SOCKET_LIMIT] = {0};
int iActiveSockNum = 0;
int iEmptySockIndex = 0;
// int iRequestNum = 0;

pthread_mutex_t lock;

void *handleConnectionTask(void *args)
{
    int iNewSocketFd = *((int *)args);
    pthread_mutex_lock(&lock);
    iActiveSockNum++;
    // iRequestNum++;

    char szMessage[1024] = {0};
    unsigned int uiMessageSize = 0;
    int iReadedByte = 0;
    int iReadedSizeMessage = 0;
    int iReadedMessageSize = 0;

    while(1){
        while(iReadedSizeMessage != 4){
            nanosleep(&request2, &remaining2);
            iReadedByte = recv(iNewSocketFd,(&uiMessageSize)+iReadedSizeMessage,(SOCK_HEADER_SIZE-iReadedSizeMessage),0);
            iReadedSizeMessage += iReadedByte;
            if(iReadedByte == 0){
                break;
            }
        }
        if(iReadedByte == 0){
            break;
        }
        printf("Message Size : %u --- ",uiMessageSize);
        while(iReadedMessageSize != uiMessageSize){
            nanosleep(&request2, &remaining2);
            iReadedByte = recv(iNewSocketFd,szMessage+iReadedMessageSize,uiMessageSize-iReadedMessageSize,0);
            iReadedMessageSize+=iReadedByte;
            if(iReadedByte == 0){
                break;
            }
        }
        if(iReadedByte == 0){
            break;
        }
        printf("Message : %s\n",szMessage);
        memset(szMessage,1024,0);
        iReadedByte = 0;
        iReadedSizeMessage = 0;
        iReadedMessageSize = 0;
        uiMessageSize = 0;
        iReadedByte = 0;
    }



    for (int i = 0; i < ACTIVE_SOCKET_LIMIT; i++)
    {
        if (iNewSocketFd == iaSocketsFd[i])
        {
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
    if (iServerFd < 0)
    {
        perror("Socket couldn't be created!\n");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(SOCK_PORT);
    address.sin_addr.s_addr = INADDR_ANY;
    socklen_t addrlen = sizeof(address);

    int iSockOpt = 1;
    if (setsockopt(iServerFd, SOL_SOCKET, SO_REUSEADDR, &iSockOpt, sizeof(iSockOpt)) < 0)
    {
        perror("setsockopt failed!\n");
        exit(EXIT_FAILURE);
    }

    if ((bind(iServerFd, (struct sockaddr *)&address, addrlen)) < 0)
    {
        perror("Socket couldn't be binded!\n");
        exit(EXIT_FAILURE);
    }

    if (listen(iServerFd, 100) < 0)
    {
        perror("Socket couldn't be listened!\n");
        exit(EXIT_FAILURE);
    }

    int iNewSocketFd;
    pthread_t tempTask;
    while (1)
    {
        while (1)
        {
            if (iActiveSockNum < 10)
            {
                break;
            }
            printf("Connection waiting!\n");
            nanosleep(&request1, &remaining1);
        }
        for (int i = 0; i < ACTIVE_SOCKET_LIMIT; i++)
        {
            if (iaSocketsFd[i] == 0)
            {
                iEmptySockIndex = i;
                break;
            }
        }
        iaSocketsFd[iEmptySockIndex] = accept(iServerFd, (struct sockaddr *)&address, &addrlen);
        if (iaSocketsFd[iEmptySockIndex] < 0)
        {
            perror("Connection couldn't be accepted!\n");
            exit(EXIT_FAILURE);
        }
        if ((pthread_create(&tempTask, NULL, handleConnectionTask, (void *)&iaSocketsFd[iEmptySockIndex])) != 0)
        {
            perror("Thread couldn't be created!\n");
            exit(EXIT_FAILURE);
        }
    }

    shutdown(iServerFd, SHUT_RDWR);
    close(iServerFd);
    return 0;
}
