#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

void printError(const char* msg)
{
    fprintf(stderr, "[ERR] %s: %s\n", msg, strerror(errno));
    exit(1);
}

int main()
{
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        printError("socket");

    struct sockaddr_in serv_addr = { 0 };
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(5000);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        printError("connect");

    char buf[1024];

    while (1)
    {
        if (fgets(buf, sizeof(buf), stdin) == NULL)
            break;

        if (strcmp(buf, "QUIT\n") == 0)
            break;

        if (send(sock, buf, strlen(buf), 0) == -1)
            printError("send");

        int len = recv(sock, buf, sizeof(buf) - 1, 0);
        if (len == -1)
            printError("recv");

        if (len == 0) {
            printf("server closed connection\n");
            break;
        }

        buf[len] = 0;
        printf("from server: %s", buf);
    }

    close(sock);
    return 0;
}