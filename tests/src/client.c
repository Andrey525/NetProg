#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

char message[] = "Hello world!\n";
char buf[sizeof(message)];

int main() {
  int sock;
  struct sockaddr_in addr;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("socket");
    exit(1);
  }

  addr.sin_family = AF_INET;
  addr.sin_port = htons(3425);
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    perror("connect");
    exit(2);
  }
  for (int i = 0; i < 10; i++) {
    send(sock, message, sizeof(message), 0);
    recv(sock, buf, sizeof(message), 0);

    printf("%s", buf);
    sleep(1);
  }

  close(sock);

  return 0;
}