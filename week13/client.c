#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8000

typedef struct {
    int type;       // 1:로그인 2:로그아웃 3:이동 4:공격 5:아이템
    int player_id;  // 서버가 로그인 응답으로 부여한 ID
    float x, y;
    int target_id;
    char username[32];
    char password[32];
} Packet;

int tcp_fd;

void *recv_loop(void *arg);
void  send_tcp(Packet pkt);
void  send_udp(int udp_fd, struct sockaddr_in *addr, Packet pkt);

int main() {
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &addr.sin_addr);

    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    connect(tcp_fd, (struct sockaddr *)&addr, sizeof(addr));

    int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);

    srand(time(NULL));
    char username[32], password[32];
    snprintf(username, sizeof(username), "user%d", rand() % 10000);
    strncpy(password, "1234", sizeof(password));

    // 로그인 (TCP) → 서버 응답으로 player_id 획득
    Packet login = {.type = 1};
    strncpy(login.username, username, 32);
    strncpy(login.password, password, 32);
    send_tcp(login);
    printf("Sent: Login (%s)\n", username);

    Packet resp;
    recv(tcp_fd, &resp, sizeof(resp), 0);
    int my_id = resp.player_id;
    printf("Assigned player_id: %d\n", my_id);

    pthread_t t;
    pthread_create(&t, NULL, recv_loop, NULL);

    usleep(500000); //0.5초

    // 이동 (UDP)
    send_udp(udp_fd, &addr, (Packet){.type = 3, .player_id = my_id, .x = 10.5f, .y = 20.3f});
    printf("Sent: Move (UDP)\n");
    usleep(500000);

    // 공격 (TCP)
    send_tcp((Packet){.type = 4, .player_id = my_id, .target_id = 1});
    printf("Sent: Attack -> 1\n");
    usleep(500000);

    // 아이템 (TCP)
    send_tcp((Packet){.type = 5, .player_id = my_id});
    printf("Sent: Item use\n");
    usleep(500000);

    // 로그아웃 (TCP)
    send_tcp((Packet){.type = 2, .player_id = my_id});
    printf("Sent: Logout\n");
    usleep(500000);

    close(tcp_fd);
    close(udp_fd);
    return 0;
}

void *recv_loop(void *arg) {
    Packet pkt;
    while (recv(tcp_fd, &pkt, sizeof(pkt), 0) > 0)
        printf("Recv: type=%d player=%d (%s) (%.1f,%.1f)\n",
               pkt.type, pkt.player_id, pkt.username, pkt.x, pkt.y);
    return NULL;
}

void send_tcp(Packet pkt) {
    send(tcp_fd, &pkt, sizeof(pkt), 0);
}

void send_udp(int udp_fd, struct sockaddr_in *addr, Packet pkt) {
    sendto(udp_fd, &pkt, sizeof(pkt), 0, (struct sockaddr *)addr, sizeof(*addr));
}
