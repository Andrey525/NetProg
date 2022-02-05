#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
  int sock;
  struct sockaddr_in client_addr, server_addr;
  struct hostent *hp;
  char buf[BUF_SIZE];

  if (argc < 4) {
    printf("ENTER ./bin/client hostname port message\n");
    exit(1);
  }

  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket");
    exit(1);
  }

  bzero((char *)&server_addr, sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  hp = gethostbyname(argv[1]);
  bcopy(hp->h_addr, &server_addr.sin_addr, hp->h_length);
  server_addr.sin_port = htons(atoi(argv[2]));

  bzero((char *)&client_addr, sizeof(client_addr));

  client_addr.sin_family = AF_INET;
  client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  client_addr.sin_port = 0;

  if (bind(sock, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
    perror("bind");
    exit(2);
  }

  printf("CLIENT: Is ready for sending\n");

  bzero(buf, sizeof(char) * BUF_SIZE);
  strcpy(buf, argv[3]);
  printf("CLIENT: Message to send: \"%s\"\n", buf);
  if (sendto(sock, buf, strlen(buf), 0, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
    perror("sendto");
    exit(3);
  }

  printf("CLIENT: Sending complete\n");

  close(sock);

  return 0;
}