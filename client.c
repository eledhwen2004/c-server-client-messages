#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <memory.h>
#define THREAD_LIMIT 10

int iaIndexes[THREAD_LIMIT];

int connectSocket()
{
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0)
    {
        perror("socket could not created!\n");
        return -1;
    }

    printf("socket created!\n");

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(4040);
    if (inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) < 1)
    {
        perror("address invalid or not supported!\n");
        return -1;
    }

    int connectionStatus = connect(socketfd, (struct sockaddr *)&address, sizeof(address));
    if (connectionStatus < 0)
    {
        perror("connection not established!\n");
        return -1;
    }
    return socketfd;
}

void *sendMessageTask(void *args)
{
    int iSocketIndex = *((int*)args);
    int socketfd = connectSocket();
    char szMessageSend[101] = {0};
    int iMsgLen;
    for (int i = 0; i < 10000; i++)
    {  
        sprintf(szMessageSend + 4, "Hello, this is message %d from Client %d!", i,socketfd);
        iMsgLen = strlen(szMessageSend+4);
        memcpy(szMessageSend,&iMsgLen,4);
        printf("Message Length : %d --- ",iMsgLen),
        memcpy(&iMsgLen,szMessageSend,4);
        printf("Message Length : %d \n",iMsgLen);
        printf("Message : %s",szMessageSend+4);
        int iRet = send(socketfd, szMessageSend, strlen(szMessageSend+4)+4, 0);
        if (iRet < 0)
        {
            perror("ERR");
            close(socketfd);
  	        exit(1);
        }
        printf("Message succesfully sent to server from client %d!\n",socketfd);
    }

    close(socketfd);
    return NULL;
}

int main()
{
    pthread_t tasks[THREAD_LIMIT] = {0};

    printf("client sockets starting...\n");
    for (int i = 0; i < THREAD_LIMIT; i++)
    {
        iaIndexes[i] = i + 1;
        if (pthread_create(&tasks[i], NULL, &sendMessageTask, (void *)(&iaIndexes[i])) < 0)
        {
            perror("thread cannot created!\n");
            return -1;
        }
    }

    for (int i = 0; i < THREAD_LIMIT; i++)
    {
        pthread_join(tasks[i], NULL);
    }

    printf("All messages sent!\n");

    return 0;
}
