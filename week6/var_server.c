#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "var_lcommon.h"

#define PORT 5000
#define BUF_SIZE 1024

int main() {
int serv_sock, clnt_sock;
struct sockaddr_in serv_adr, clnt_adr;
socklen_t clnt_adr_sz;

serv_sock = socket(PF_INET, SOCK_STREAM, 0);
if (serv_sock == -1)
error_handling("socket() error");

memset(&serv_adr, 0, sizeof(serv_adr));
serv_adr.sin_family = AF_INET;
serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
serv_adr.sin_port = htons(PORT);

if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
error_handling("bind() error");

if (listen(serv_sock, 5) == -1)
error_handling("listen() error");

clnt_adr_sz = sizeof(clnt_adr);
printf("Linux Variable Text Server started on port %d. Waiting for client...\n", PORT);

clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
if (clnt_sock == -1)
error_handling("accept() error");

char buffer[BUF_SIZE];
struct DrawingMessage msg = {0, 0, 0, 0, 0, 0};
int recv_len;

printf("Receiving data...\n");

while ((recv_len = read(clnt_sock, buffer, BUF_SIZE - 1)) > 0) {
buffer[recv_len] = '\0';
// "END" 마커 확인
if (strstr(buffer, "END") != NULL) {
printf("Message reception complete.\n");
break;
}

//가변 길이 데이터 수신
// key=value 파싱 (Json과 유사한 형식)
char *line = strtok(buffer, "\n");
while (line != NULL) {
if (strstr(line, "x1=") == line) sscanf(line, "x1=%d", &msg.x1);
else if (strstr(line, "y1=") == line) sscanf(line, "y1=%d", &msg.y1);
else if (strstr(line, "x2=") == line) sscanf(line, "x2=%d", &msg.x2);
else if (strstr(line, "y2=") == line) sscanf(line, "y2=%d", &msg.y2);
else if (strstr(line, "width=") == line) sscanf(line, "width=%d", &msg.width);
else if (strstr(line, "color=") == line) sscanf(line, "color=%d", &msg.color);
line = strtok(NULL, "\n");
}
}

printf("Parsed Message:\n");
printf("P1: (%d, %d), P2: (%d, %d)\n", msg.x1, msg.y1, msg.x2, msg.y2);
printf("Width: %d, Color: %x\n", msg.width, msg.color);

close(clnt_sock);
close(serv_sock);
return 0;
}