#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
  int sock, sock_listen;
  struct sockaddr_in addr;
  char buf[1024];
  int bytes_read;

  sock_listen = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_listen < 0) {
    perror("sock");
    exit(1);
  }

  addr.sin_family = AF_INET;
  addr.sin_port = htons(3425);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(sock_listen, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    perror("bind");
    exit(2);
  }

  listen(sock_listen, 1);

  while (1) {
    sock = accept(sock_listen, NULL, NULL);
    if (sock < 0) {
      perror("accpet");
      exit(3);
    }

    while (1) {
      bytes_read = recv(sock, buf, 1024, 0);
      if (bytes_read <= 0) {
        break;
      }
      send(sock, buf, bytes_read, 0);
    }
    close(sock);
  }

  return 0;
}