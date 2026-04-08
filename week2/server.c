#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main()
{
    int sock = socket(PF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr = { 0 };
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000);

    bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    listen(sock, 5);

    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_len = sizeof(clnt_addr);
    int csock = accept(sock, (struct sockaddr*)&clnt_addr, &clnt_addr_len);

    char buf[1024];
    int str_len;
    while((str_len = read(csock, buf, 1024)) != 0) {
        buf[str_len] = 0;
        printf("%s\n", buf);
        write(csock, buf, str_len);
    }
    close(csock);
    close(sock);
    return 0;
}

//전체 구조 잡기
//sock 만들기
//서버 주소 구조체 만들기
//소켓과 주소 구조체 연결하기(bind)
//lisen으로 클라의 연결 대기
//클라 요청시 accept로 연결 수락
//클라로부터 명령어 받기 (read)
//명령어 출력하기
//클라로 명령어 보내기 (write)
//소켓 닫기 (close)