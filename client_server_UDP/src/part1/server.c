#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 1024

int main() {
    int sock;
    socklen_t length;
    int bytes_read;
    struct sockaddr_in server_addr, client_addr;
    char *buf = malloc(sizeof(char) * BUF_SIZE);

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = 0;

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(2);
    }
    length = sizeof(server_addr);
    if (getsockname(sock, (struct sockaddr *)&server_addr, &length) < 0) {
        perror("getsockname");
        exit(3);
    }
    printf("SERVER: port number - %d\n", ntohs(server_addr.sin_port));

    while (1) {
        memset(buf, 0, sizeof(char) * BUF_SIZE);
        length = sizeof(struct sockaddr_in);
        if ((bytes_read = recvfrom(sock, buf, BUF_SIZE, 0,
                                   (struct sockaddr *)&client_addr, &length)) <
            0) {
            perror("recvfrom");
            exit(4);
        }

        printf("SERVER: Client address: %s:%d\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        printf("SERVER: Received message: \"%s\"\n", buf);

        buf = strcat(buf, "!");
        if (sendto(sock, buf, strlen(buf), 0, (struct sockaddr *)&client_addr,
                   sizeof(client_addr)) < 0) {
            perror("sendto");
            exit(5);
        }
        printf("SERVER: Message was changed to: \"%s\"\n", buf);

        printf("SERVER: Reverse sending complete\n");
    }

    close(sock);
    return 0;
}