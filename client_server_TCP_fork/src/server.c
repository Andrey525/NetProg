#include <arpa/inet.h>
#include <memory.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_QUEUE 10

int main() {
    signal(SIGCHLD,
           SIG_IGN); // после завершения процессы не преобразовываются в зомби
    int sock_client, sock_server;
    socklen_t length;
    struct sockaddr_in server_addr, client_addr;
    int n;
    int bytes_read;
    pid_t pid;

    if ((sock_server = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = 0;

    if (bind(sock_server, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("bind");
        exit(2);
    }
    length = sizeof(server_addr);
    if (getsockname(sock_server, (struct sockaddr *)&server_addr, &length) <
        0) {
        perror("getsockname");
        exit(3);
    }
    printf("SERVER: port number - %d\n", ntohs(server_addr.sin_port));

    listen(sock_server, MAX_QUEUE);

    while (1) {
        if ((sock_client = accept(sock_server, (struct sockaddr *)&client_addr,
                                  &length)) < 0) {
            perror("accept");
            exit(4);
        }

        pid = fork();
        if (pid == 0) {
            close(sock_server);
            while (1) {
                if ((bytes_read = recv(sock_client, &n, sizeof(int), 0)) <= 0) {
                    break;
                }
                printf("SERVER: Client address: %s:%d\n",
                       inet_ntoa(client_addr.sin_addr),
                       ntohs(client_addr.sin_port));
                printf("SERVER: Received n = %d\n", n);
            }
            close(sock_client);
            exit(EXIT_SUCCESS);
        } else if (pid < 0) {
            perror("fork");
            exit(5);
        } else {
            close(sock_client);
        }
    }

    close(sock_server);
    return 0;
}