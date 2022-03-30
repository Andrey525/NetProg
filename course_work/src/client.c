#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int sock;
struct sockaddr_in server_addr;

void clear_stdin() {
    while (getchar() != '\n')
        ;
}

void *input() {
    char message[256];
    int bytes_send;
    while (1) {
        do {
            fgets(message, 256, stdin);
            if (message[strlen(message) - 1] == '\n') {
                message[strlen(message) - 1] = '\0';
            } else {
                clear_stdin();
            }
        } while (message[0] == '\0');
        if ((bytes_send = send(sock, &message, sizeof(message), 0)) <= 0) {
            perror("CLIENT: send");
            exit(1);
        }
    }
}

void output() {
    int bytes_read;
    char message[256];
    while (1) {
        if ((bytes_read = recv(sock, message, sizeof(message), 0)) <= 0) {
            break;
        }
        printf("Anonim: %s\n", message);
    }
}

int main() {
    pthread_t thread;
    int status;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("CLIENT: socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server_addr.sin_port = htons(8080);

    status =
        connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (status != 0) {
        perror("CLIENT: connect");
        exit(1);
    }
    printf("CLIENT: Is ready for sending\n");
    printf("CLIENT: Sending...\n");

    status = pthread_create(&thread, NULL, input, NULL);
    if (status != 0) {
        perror("CLIENT: pthread_create");
        exit(1);
    }
    status = pthread_detach(thread);
    if (status != 0) {
        perror("CLIENT: pthread_detach");
        exit(1);
    }
    output();
    close(sock);
    return 0;
}