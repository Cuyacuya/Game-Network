#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define BUF_SIZE 1024
#define NAME_SIZE 30

char name[NAME_SIZE] = "[Default]";
char msg[BUF_SIZE];

void *send_msg(void *arg);
void *recv_msg(void *arg);

int main() {
    printf("Enter your name : ");
    fgets(name, NAME_SIZE, stdin);
    name[strcspn(name, "\n")] = 0; // 엔터 제거

    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(5000);   // htonl 아님, htons

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("connect");
        close(sock);
        return 1;
    }

    pthread_t sender, receiver;

    pthread_create(&sender, NULL, send_msg, (void*)&sock);
    pthread_create(&receiver, NULL, recv_msg, (void*)&sock);

    pthread_join(sender, NULL);
    pthread_join(receiver, NULL);

    close(sock);
    return 0;
}

void *send_msg(void *arg) {
    int sock = *(int*)arg;
    char name_msg[NAME_SIZE + BUF_SIZE + 2];

    while (1) {
        printf("Input message: ");
        fflush(stdout);

        if (fgets(msg, BUF_SIZE, stdin) == NULL)
            break;

        if (!strcmp(msg, "q\n")) {
            close(sock);
            exit(0);
        }

        snprintf(name_msg, sizeof(name_msg), "[%s] %s", name, msg);
        write(sock, name_msg, strlen(name_msg));
    }

    return NULL;
}

void *recv_msg(void *arg) {
    int sock = *(int*)arg;
    char name_msg[NAME_SIZE + BUF_SIZE];
    int len;

    while (1) {
        len = read(sock, name_msg, NAME_SIZE + BUF_SIZE - 1);
        if (len <= 0)
            return NULL;

        name_msg[len] = '\0';   // 문자열 끝 표시 꼭 필요

        printf("\r%s", name_msg);
        printf("Input message: ");
        fflush(stdout);
    }

    return NULL;
}