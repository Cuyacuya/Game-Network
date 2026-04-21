#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>

void printError(const char* msg)
{
    fprintf(stderr, "[ERR] %s: %s\n", msg, strerror(errno));
    exit(1);
}

int main() {
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1)
        printError("socket");

    struct sockaddr_in serv_addr = { 0 };
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000);

    if(bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        printError("bind");

    if(listen(sock, 5) == -1)
        printError("listen");

    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_len = sizeof(clnt_addr);
    int csock = accept(sock, (struct sockaddr*)&clnt_addr, &clnt_addr_len);
    if(csock == -1)
        printError("accept");

    char buf[1024];
    int str_len;
    char response[1024];

    while(1){
        str_len = recv(csock, buf, 1024, 0);

        if(str_len == -1)
        printError("recv");

        if(str_len == 0) //종료 
        break;

        buf[str_len] = 0; //recv는 문자열 끝에 \0을 붙여주지 못함.

        if(strcmp(buf, "TIME\n") == 0) {
            time_t nowT = time(NULL);
            struct tm tm = *localtime(&nowT);

            strftime(response, sizeof(response), "%Y-%m-%d %H:%M:%S\n", &tm); //시간을 원하는 형식의 문자열로 변환
                
            if(send(csock, response, strlen(response), 0) == -1)
                printError("send");
        }

        else if(strncmp(buf, "NAME ", 5) == 0 ){
            char *name = buf + 5;
            name[strcspn(name, "\n")] = 0; //특정 문자가 처음 나오는 위치 반환

            sprintf(response, "My name is %s's server\n", name);

            if(send(csock, response, strlen(response), 0) == -1)
                printError("send");
        }
    }
    close(csock);
    close(sock);
    return 0;
}