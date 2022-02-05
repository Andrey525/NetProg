#include <arpa/inet.h>
#include <memory.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int sock;
  struct sockaddr_in client_addr, server_addr;
  struct hostent *hp;
  int i, n;

  if (argc < 4) {
    printf("ENTER ./bin/client hostname port n\n");
    exit(1);
  }

  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket");
    exit(1);
  }

  memset(&server_addr, 0, sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  hp = gethostbyname(argv[1]);
  bcopy(hp->h_addr, &server_addr.sin_addr, hp->h_length);
  server_addr.sin_port = htons(atoi(argv[2]));

  memset(&client_addr, 0, sizeof(client_addr));

  client_addr.sin_family = AF_INET;
  client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  client_addr.sin_port = 0;

  if (bind(sock, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
    perror("bind");
    exit(2);
  }

  printf("CLIENT: Is ready for sending\n");
  printf("CLIENT: Sending...\n");
  n = atoi(argv[3]);
  for (i = 0; i < n; i++) {
    if (sendto(sock, &n, sizeof(int), 0, (struct sockaddr *)&server_addr,
               sizeof(server_addr)) < 0) {
      perror("sendto");
      exit(3);
    }
    printf("CLIENT: Sent %d from %d\n", i + 1, n);
    sleep(n);
  }

  printf("CLIENT: Sending complete\n");

  close(sock);

  return 0;
}