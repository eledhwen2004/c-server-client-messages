/* #include <stdio.h>
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
} */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <pthread.h>
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
    int iSocketIndex = *((int *)args);
    int socketfd = connectSocket();
    int iMessageSize = 0;
    char szMessageReceived[100] = {0};
    char szMessageSend[101] = {0};
    for (int i = 0; i < 10000; i++)
    {
        sprintf(szMessageSend, "Hello, this is message %d from Client %d!\n", i, iSocketIndex);
        int iRet = send(socketfd, szMessageSend, strlen(szMessageSend), 0);
        if (iRet < 0)
        {
            perror("ERR");
            close(socketfd);
            exit(1);
        }
        printf("Message succesfully sent from client!\n");
        iMessageSize = recv(socketfd, &szMessageReceived[0], 100, 0);
        if(iMessageSize == 0){
            printf("Approve Message didin't come!\n");
            break;
        }
        printf("Approve Message : %s\n", szMessageReceived);
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