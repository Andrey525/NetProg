#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_COUNT_CLIENTS 10

pthread_mutex_t MUTEX = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MUTEX_FOR_LIST_OF_CLIENTS = PTHREAD_MUTEX_INITIALIZER;

int count_clients = 0;
int clients[MAX_COUNT_CLIENTS];

void *process() {
    int bytes_read, bytes_send;
    char message[256];
    int status;
    int sock_client;

    sock_client = clients[count_clients - 1];
    status = pthread_mutex_unlock(&MUTEX);
    if (status != 0) {
        perror("SERVER: pthread_mutex_unlock");
        exit(1);
    }

    while (1) {
        bytes_read = recv(sock_client, message, sizeof(message), 0);
        if (bytes_read == 0) {
            status = pthread_mutex_lock(&MUTEX_FOR_LIST_OF_CLIENTS);
            if (status != 0) {
                perror("SERVER: pthread_mutex_lock");
                exit(1);
            }
            for (int i = 0; i < count_clients; i++) {
                if (sock_client == clients[i]) {
                    if (count_clients > 1) {
                        clients[i] = clients[count_clients - 1];
                        clients[count_clients - 1] = 0;
                        count_clients--;
                    } else {
                        clients[i] = 0;
                        count_clients = 0;
                    }
                }
            }
            status = pthread_mutex_unlock(&MUTEX_FOR_LIST_OF_CLIENTS);
            if (status != 0) {
                perror("SERVER: pthread_mutex_unlock");
                exit(1);
            }
            close(sock_client);
            pthread_exit(NULL);
        } else if (bytes_read < 0) {
            perror("SERVER: recv");
            exit(1);
        }
        printf("SERVER: Received message = %s\n", message);

        status = pthread_mutex_lock(&MUTEX_FOR_LIST_OF_CLIENTS);
        if (status != 0) {
            perror("SERVER: pthread_mutex_lock");
            exit(1);
        }
        for (int i = 0; i < count_clients; i++) {
            if (sock_client != clients[i]) {
                if ((bytes_send =
                         send(clients[i], &message, sizeof(message), 0)) <= 0) {
                    perror("SERVER: send");
                    exit(1);
                }
            }
        }
        status = pthread_mutex_unlock(&MUTEX_FOR_LIST_OF_CLIENTS);
        if (status != 0) {
            perror("SERVER: pthread_mutex_unlock");
            exit(1);
        }
    }
}

int main() {
    int sock_client, sock_server;
    socklen_t length;
    struct sockaddr_in server_addr, client_addr;
    pthread_t thread;
    int status;

    if ((sock_server = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("SERVER: socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(struct sockaddr_in));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server_addr.sin_port = htons(8080);

    status = bind(sock_server, (struct sockaddr *)&server_addr,
                  sizeof(struct sockaddr_in));
    if (status != 0) {
        perror("SERVER: bind");
        exit(1);
    }

    if (listen(sock_server, 5) < 0) {
        perror("SERVER: listen");
        exit(1);
    }

    while (1) {
        length = sizeof(struct sockaddr_in);

        while (count_clients == MAX_COUNT_CLIENTS) {
            sleep(0.1);
        }

        if ((sock_client = accept(sock_server, (struct sockaddr *)&client_addr,
                                  &length)) < 0) {
            perror("SERVER: accept");
            exit(1);
        }

        status = pthread_mutex_lock(&MUTEX);
        if (status != 0) {
            perror("SERVER: pthread_mutex_lock");
            exit(1);
        }

        status = pthread_mutex_lock(&MUTEX_FOR_LIST_OF_CLIENTS);
        if (status != 0) {
            perror("SERVER: pthread_mutex_lock");
            exit(1);
        }
        count_clients++;
        clients[count_clients - 1] = sock_client;
        status = pthread_mutex_unlock(&MUTEX_FOR_LIST_OF_CLIENTS);
        if (status != 0) {
            perror("SERVER: pthread_mutex_unlock");
            exit(1);
        }

        status = pthread_create(&thread, NULL, process, NULL);
        if (status != 0) {
            perror("SERVER: pthread_create");
            exit(1);
        }
        status = pthread_detach(thread);
        if (status != 0) {
            perror("SERVER: pthread_detach");
            exit(1);
        }
    }
    close(sock_server);
    return 0;
}