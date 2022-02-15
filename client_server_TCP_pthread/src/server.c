#include <arpa/inet.h>
#include <fcntl.h>
#include <memory.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_QUEUE 10
#define BUF_SIZE 256
#define SOCKET_SOCKET_ERROR -1
#define SOCKET_BIND_ERROR -2
#define SOCKET_GETSOCKNAME_ERROR -3
#define SOCKET_ACCEPT_ERROR -4
#define SOCKET_LISTEN_ERROR -5
#define PTHREAD_CREATE_ERROR -10
#define PTHREAD_DETACH_ERROR -11
#define PTHREAD_MUTEX_LOCK_ERROR -12
#define PTHREAD_MUTEX_UNLOCK_ERROR -13
#define FILE_OPEN_ERROR -20
#define FILE_WRITE_ERROR -21

typedef struct {
    int sock_client;
    struct sockaddr_in client_addr;
    int fd;
} PthreadData;

pthread_mutex_t MUTEX = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MUTEXFILE = PTHREAD_MUTEX_INITIALIZER;

void *process(void *inputData) {
    int bytes_read;
    int n;
    char *buf;
    int fd;
    int status;
    int sock_client;
    struct sockaddr_in client_addr;

    PthreadData *data = (PthreadData *)inputData;
    sock_client = data->sock_client;
    client_addr = data->client_addr;
    fd = data->fd;

    buf = malloc(BUF_SIZE * sizeof(char));
    memset(buf, 0, BUF_SIZE * sizeof(char));

    status = pthread_mutex_unlock(&MUTEX);
    if (status != 0) {
        perror("pthread_mutex_unlock");
        exit(PTHREAD_MUTEX_UNLOCK_ERROR);
    }

    while (1) {
        if ((bytes_read = recv(sock_client, &n, sizeof(int), 0)) <= 0) {
            break;
        }
        printf("SERVER: Client address: %s:%d\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        printf("SERVER: Received n = %d\n", n);

        status = pthread_mutex_lock(&MUTEXFILE);
        if (status != 0) {
            perror("pthread_mutex_lock");
            exit(PTHREAD_MUTEX_LOCK_ERROR);
        }
        status =
            sprintf(buf, "%s %s%c%d\n%s %d\n%s\n",
                    "SERVER: Client address:", inet_ntoa(client_addr.sin_addr),
                    ':', ntohs(client_addr.sin_port),
                    "SERVER: Received n = ", n, "___________________");
        if (status < 0) {
            perror("sprintf");
            exit(FILE_WRITE_ERROR);
        }
        if (write(fd, buf, strlen(buf)) == -1) {
            perror("write");
            exit(FILE_WRITE_ERROR);
        }

        status = pthread_mutex_unlock(&MUTEXFILE);
        if (status != 0) {
            perror("pthread_mutex_unlock");
            exit(PTHREAD_MUTEX_UNLOCK_ERROR);
        }
    }
    close(sock_client);
    pthread_exit(NULL);
}

int main() {
    int sock_client, sock_server;
    socklen_t length;
    struct sockaddr_in server_addr, client_addr;
    pthread_t thread;
    PthreadData data;
    int status;
    int fd;

    fd = open("data.txt", O_CREAT | O_EXCL | O_WRONLY, S_IRWXU);
    if (fd == -1) {
        fd = open("data.txt", O_WRONLY | O_TRUNC, S_IRWXU);
        if (fd == -1) {
            perror("open");
            exit(FILE_OPEN_ERROR);
        }
    }

    if ((sock_server = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(SOCKET_SOCKET_ERROR);
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = 0;

    if (bind(sock_server, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("bind");
        exit(SOCKET_BIND_ERROR);
    }
    length = sizeof(server_addr);
    if (getsockname(sock_server, (struct sockaddr *)&server_addr, &length) <
        0) {
        perror("getsockname");
        exit(SOCKET_GETSOCKNAME_ERROR);
    }
    printf("SERVER: port number - %d\n", ntohs(server_addr.sin_port));

    if (listen(sock_server, MAX_QUEUE) < 0) {
        perror("listen");
        exit(SOCKET_LISTEN_ERROR);
    }

    while (1) {
        if ((sock_client = accept(sock_server, (struct sockaddr *)&client_addr,
                                  &length)) < 0) {
            perror("accept");
            exit(SOCKET_ACCEPT_ERROR);
        }

        status = pthread_mutex_lock(&MUTEX);
        if (status != 0) {
            perror("pthread_mutex_unlock");
            exit(PTHREAD_MUTEX_UNLOCK_ERROR);
        }

        data.sock_client = sock_client;
        data.client_addr = client_addr;
        data.fd = fd;

        status = pthread_create(&thread, NULL, process, &data);
        if (status != 0) {
            perror("pthread_create");
            exit(PTHREAD_CREATE_ERROR);
        }
        status = pthread_detach(thread);
        if (status != 0) {
            perror("pthread_detach");
            exit(PTHREAD_DETACH_ERROR);
        }
    }
    close(fd);
    close(sock_server);
    return 0;
}