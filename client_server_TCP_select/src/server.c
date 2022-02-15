#include <arpa/inet.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
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
#define SOCKET_RECV_ERROR -6
#define SELECT_ERROR -10

int receive_data(int fd, struct sockaddr_in client_addr) {
    int bytes_read;
    int n;
    bytes_read = recv(fd, &n, sizeof(int), 0);
    if (bytes_read < 0) {
        perror("recv");
        exit(SOCKET_RECV_ERROR);
    } else if (bytes_read == 0) {
        return 1;
    }
    printf("SERVER: Client address: %s:%d\n", inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port));
    printf("SERVER: Received n = %d\n", n);
    return 0;
}

int main() {
    int sock_client, sock_server;
    socklen_t length;
    struct sockaddr_in server_addr, tmp_addr, client_addr[FD_SETSIZE];
    fd_set readfds, activefds;
    int fd;

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

    FD_ZERO(&activefds);
    FD_SET(sock_server, &activefds);
    while (1) {
        memcpy(&readfds, &activefds, sizeof(readfds));
        if (select(FD_SETSIZE, &readfds, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(SELECT_ERROR);
        }

        if (FD_ISSET(sock_server, &readfds)) {
            if ((sock_client = accept(sock_server, (struct sockaddr *)&tmp_addr,
                                      &length)) < 0) {
                perror("accept");
                exit(SOCKET_ACCEPT_ERROR);
            }
            memcpy(&client_addr[sock_client], &tmp_addr, sizeof(client_addr));
            FD_SET(sock_client, &activefds);
        }

        for (fd = 0; fd < FD_SETSIZE; fd++) {
            if (fd != sock_server && FD_ISSET(fd, &readfds)) {
                if (receive_data(fd, client_addr[fd])) {
                    close(fd);
                    FD_CLR(fd, &activefds);
                    memset(&client_addr[fd], 0,
                           sizeof(sizeof(struct sockaddr_in)));
                }
            }
        }
    }
    close(sock_server);
    return 0;
}