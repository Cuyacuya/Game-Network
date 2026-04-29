#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int sock = socket(PF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr = { 0 };
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(5000);

    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    char buf[1024];
    while(1){
        fgets(buf, 1024, stdin);
        if(strcmp(buf, "q\n") == 0 ) break;

        write(sock, buf, (int)strlen(buf));

        int len = (int)read(sock, buf, 1024);
        buf[len] = 0;
        printf("from server : %s\n", buf);
    }
    close(sock);
    return 0;
}