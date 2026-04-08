#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "var_lcommon.h"

#define SERVER_IP "127.0.0.1"
#define PORT 5000
#define BUF_SIZE 1024

int main() {
int sock;
struct sockaddr_in serv_adr;

sock = socket(PF_INET, SOCK_STREAM, 0);
if (sock == -1)
error_handling("socket() error");

memset(&serv_adr, 0, sizeof(serv_adr));
serv_adr.sin_family = AF_INET;
serv_adr.sin_addr.s_addr = inet_addr(SERVER_IP);
serv_adr.sin_port = htons(PORT);

if (connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
error_handling("connect() error!");

struct DrawingMessage msg = {10, 20, 100, 200, 5, 0xFF0000};
char buffer[BUF_SIZE];

// 구조체 데이터를 key=value 형식의 문자열로 변환
sprintf(buffer, "x1=%d\ny1=%d\nx2=%d\ny2=%d\nwidth=%d\ncolor=%d\nEND\n",
msg.x1, msg.y1, msg.x2, msg.y2, msg.width, msg.color);

// 가변 길이 데이터 전송
write(sock, buffer, strlen(buffer));
printf("Variable Text Message sent to server (%s:%d):\n%s\n", SERVER_IP, PORT, buffer);

close(sock);
return 0;
}