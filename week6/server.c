#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "common.h"

int main() {
int serv_sock = socket(PF_INET, SOCK_STREAM, 0);
if (serv_sock == -1)
error_handling("socket() error");

struct sockaddr_in serv_adr;
memset(&serv_adr, 0, sizeof(serv_adr));
serv_adr.sin_family = AF_INET;
serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
serv_adr.sin_port = htons(PORT);

if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
error_handling("bind() error");

if (listen(serv_sock, 5) == -1)
error_handling("listen() error");

printf("Linux Server started on port %d. Waiting for client...\n", PORT);

struct sockaddr_in clnt_adr;
socklen_t clnt_adr_sz = sizeof(clnt_adr);

int clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
if (clnt_sock == -1)
error_handling("accept() error");

struct DrawingMessage msg;
int str_len;

if ((str_len = read(clnt_sock, &msg, sizeof(msg))) != 0) {
msg.x1 = ntohl(msg.x1);
msg.y1 = ntohl(msg.y1);
msg.x2 = ntohl(msg.x2);
msg.y2 = ntohl(msg.y2);
msg.width = ntohl(msg.width);
msg.color = ntohl(msg.color);

printf("Received Message:\n");
printf("P1: (%d, %d), P2: (%d, %d)\n", msg.x1, msg.y1, msg.x2, msg.y2);
printf("Width: %d, Color: %x\n", msg.width, msg.color);
}

close(clnt_sock);
close(serv_sock);
return 0;
}