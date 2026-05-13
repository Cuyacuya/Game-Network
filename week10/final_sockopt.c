#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void error_handling(const char *msg) {
    perror(msg);
    exit(1);
}

int main() {
    int serv_sock;
    struct sockaddr_in serv_adr;
    
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");

    int reuse_opt = 1;
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&reuse_opt, sizeof(reuse_opt));

    int buf_size = 1024 * 5;
    setsockopt(serv_sock, SOL_SOCKET, SO_SNDBUF, (void*)&buf_size, sizeof(buf_size));
    setsockopt(serv_sock, SOL_SOCKET, SO_RCVBUF, (void*)&buf_size, sizeof(buf_size));

    int keepalive_opt = 1;
    setsockopt(serv_sock, SOL_SOCKET, SO_KEEPALIVE, (void*)&keepalive_opt, sizeof(keepalive_opt));

    printf("[SUCCESS] 서버 소켓의 주요 옵션들이 설정되었습니다.\n");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(9000);

    if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");

    printf("[SUCCESS] 포트 9000번에 bind 완료.\n");
    
    close(serv_sock);
    return 0;
}