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
    int option = 1;
    socklen_t optlen;

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");

    optlen = sizeof(option);
    if (setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&option, optlen) == -1)
        error_handling("setsockopt() error");
        
    printf("SO_REUSEADDR 옵션이 활성화되었습니다.\n");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(8080);

    if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");

    printf("bind() 성공! 서버가 8080 포트를 점유했습니다.\n");
    printf("이 프로그램을 강제로 종료(Ctrl+C)한 뒤 즉시 다시 실행해도 에러가 나지 않습니다.\n");

    close(serv_sock);
    return 0;
}