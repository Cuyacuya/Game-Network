#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int sock = socket(PF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr = {0};
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
    while((str_len = read(csock, buf, 1024)) != 0){
        buf[str_len] = 0;
        printf("%s\n", buf);
        write(csock, buf, str_len);
    }
    close(csock);
    close(sock);
    return 0;
}