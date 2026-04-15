#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAX_CLIENT 100
#define BUF_SIZE 1024

int client_count = 0;
int client_socks[MAX_CLIENT];
pthread_mutex_t mutx; // client_socks 보호용 mutex

void send_msg(char* msg, int len);
void* handle_clnt(void* arg);
void error_handling(const char* msg);

// 다른 sock이 들어올 때 main의 지역변수를 넘기면 꼬일 수 있으니
// heap에 따로 저장해서 스레드에 전달

int main() {
    pthread_mutex_init(&mutx, NULL);

    int serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000);

    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");

    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    while (1) {
        struct sockaddr_in clnt_addr;
        socklen_t clnt_addr_size = sizeof(clnt_addr);

        int clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        if (clnt_sock == -1)
            continue;

        pthread_mutex_lock(&mutx);
        if (client_count >= MAX_CLIENT) {
            pthread_mutex_unlock(&mutx);
            close(clnt_sock);
            continue;
        }
        client_socks[client_count++] = clnt_sock;
        pthread_mutex_unlock(&mutx);

        int* arg = malloc(sizeof(int)); // heap에 넣기
        if (arg == NULL) {
            close(clnt_sock);

            pthread_mutex_lock(&mutx);
            client_count--;
            pthread_mutex_unlock(&mutx);

            continue;
        }
        *arg = clnt_sock;

        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_clnt, arg) != 0) {
            free(arg);
            close(clnt_sock);

            pthread_mutex_lock(&mutx);
            client_count--;
            pthread_mutex_unlock(&mutx);

            continue;
        }

        pthread_detach(tid); // join 없이 종료 시 자원 자동 회수
        printf("connected from %s:%d\n",
               inet_ntoa(clnt_addr.sin_addr),
               ntohs(clnt_addr.sin_port));
    }

    close(serv_sock);
    pthread_mutex_destroy(&mutx);
    return 0;
}

void* handle_clnt(void* arg) {
    int clnt_sock = *(int*)arg; // heap에 넣어둔 소켓 번호 꺼내기
    free(arg); // heap 해제

    int len;
    char buf[BUF_SIZE];

    while ((len = read(clnt_sock, buf, sizeof(buf))) > 0) { // 0이면 연결 종료
        send_msg(buf, len);
    }

    pthread_mutex_lock(&mutx);
    for (int i = 0; i < client_count; i++) {
        if (clnt_sock == client_socks[i]) {
            for (int j = i; j < client_count - 1; j++) {
                client_socks[j] = client_socks[j + 1];
            }
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&mutx);

    close(clnt_sock);
    printf("client disconnected\n");

    return NULL;
}

void send_msg(char* msg, int len) {
    pthread_mutex_lock(&mutx);
    for (int i = 0; i < client_count; i++) { // 모든 클라이언트에게 전송
        write(client_socks[i], msg, len);
    }
    pthread_mutex_unlock(&mutx);
}

void error_handling(const char* msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}