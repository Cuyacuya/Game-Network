#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "common.h"

#define SERVER_IP "127.0.0.1"

int main() {
int sock = socket(PF_INET, SOCK_STREAM, 0);
if (sock == -1)
error_handling("socket() error");

struct sockaddr_in serv_adr;
memset(&serv_adr, 0, sizeof(serv_adr));
serv_adr.sin_family = AF_INET;
serv_adr.sin_addr.s_addr = inet_addr(SERVER_IP);
serv_adr.sin_port = htons(PORT);

// 서버 연결
if (connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
error_handling("connect() error!");

struct DrawingMessage msg;
msg.x1 = htonl(10);
msg.y1 = htonl(20);
msg.x2 = htonl(100);
msg.y2 = htonl(200);
msg.width = htonl(5);
msg.color = htonl(0xFF0000);

// 데이터 전송
write(sock, &msg, sizeof(msg));
printf("Message sent to server (%s:%d).\n", SERVER_IP, PORT);

close(sock);
return 0;
}