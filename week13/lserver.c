#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#define MAX_EVENTS  1000
#define MAX_PLAYERS 1000
#define PORT        8000

typedef struct {
    float x, y;
    int hp;
    int is_active;
    int tcp_fd;
    struct sockaddr_in udp_addr;
    char username[32];
} Player;

typedef struct {
    int type;       // 1:로그인 2:로그아웃 3:이동 4:공격 5:아이템
    int player_id;  // 서버가 로그인 응답으로 부여한 ID
    float x, y;
    int target_id;
    char username[32];
    char password[32];
} Packet;

Player players[MAX_PLAYERS];

int  alloc_player(int fd);
void free_player(int fd);
void broadcast_tcp(Packet *pkt);
void broadcast_udp(int udp_sock, Packet *pkt);
void on_tcp(Packet *pkt, int fd);
void on_udp(int udp_sock, Packet *pkt, struct sockaddr_in *src);

int main() {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        players[i].is_active = 0;
        players[i].tcp_fd = -1;
    }

    int tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in addr = {0};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(PORT);

    bind(tcp_sock, (struct sockaddr *)&addr, sizeof(addr));
    listen(tcp_sock, 5);
    bind(udp_sock, (struct sockaddr *)&addr, sizeof(addr));

    int epfd = epoll_create1(0);
    struct epoll_event ev, events[MAX_EVENTS];

    ev.events = EPOLLIN;
    ev.data.fd = tcp_sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, tcp_sock, &ev);
    ev.data.fd = udp_sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, udp_sock, &ev);

    printf("Server started. Port: %d\n", PORT);

    while (1) {
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;
            if (fd == tcp_sock) {
                int cfd = accept(tcp_sock, NULL, NULL);
                ev.events  = EPOLLIN;
                ev.data.fd = cfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
            } else if (fd == udp_sock) {
                Packet pkt;
                struct sockaddr_in src;
                socklen_t slen = sizeof(src);
                recvfrom(udp_sock, &pkt, sizeof(pkt), 0, (struct sockaddr *)&src, &slen);
                on_udp(udp_sock, &pkt, &src);
            } else {
                Packet pkt;
                int len = recv(fd, &pkt, sizeof(pkt), 0);
                if (len <= 0) {
                    free_player(fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                } else {
                    on_tcp(&pkt, fd);
                }
            }
        }
    }
}

int alloc_player(int fd) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!players[i].is_active) {
            players[i].is_active = 1;
            players[i].hp       = 100;
            players[i].tcp_fd   = fd;
            return i;
        }
    }
    return -1;
}

void free_player(int fd) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (players[i].tcp_fd == fd) {
            players[i].is_active = 0;
            players[i].tcp_fd    = -1;
            printf("Logout (disconnect): %d\n", i);
            return;
        }
    }
}

void broadcast_tcp(Packet *pkt) {
    for (int i = 0; i < MAX_PLAYERS; i++)
        if (players[i].is_active && players[i].tcp_fd != -1)
            send(players[i].tcp_fd, pkt, sizeof(Packet), 0);
}

void broadcast_udp(int udp_sock, Packet *pkt) {
    for (int i = 0; i < MAX_PLAYERS; i++)
        if (players[i].is_active && players[i].udp_addr.sin_family == AF_INET)
            sendto(udp_sock, pkt, sizeof(Packet), 0,
                   (struct sockaddr *)&players[i].udp_addr, sizeof(struct sockaddr_in));
}

void on_tcp(Packet *pkt, int fd) {
    int pid = pkt->player_id;
    switch (pkt->type) {
        case 1: { // 로그인: 서버가 ID 부여 후 응답
            pid = alloc_player(fd);
            if (pid < 0) return;
            strncpy(players[pid].username, pkt->username, 32);
            Packet resp    = *pkt;
            resp.player_id = pid;
            send(fd, &resp, sizeof(resp), 0);
            printf("Login: %d (%s)\n", pid, players[pid].username);
            broadcast_tcp(&resp);
            break;
        }
        case 2: // 로그아웃
            if (pid < 0 || pid >= MAX_PLAYERS) return;
            players[pid].is_active = 0;
            players[pid].tcp_fd    = -1;
            printf("Logout: %d\n", pid);
            broadcast_tcp(pkt);
            break;
        case 4: // 공격
            printf("Attack: %d -> %d\n", pid, pkt->target_id);
            broadcast_tcp(pkt);
            break;
        case 5: // 아이템
            printf("Item: %d used item\n", pid);
            broadcast_tcp(pkt);
            break;
    }
}

void on_udp(int udp_sock, Packet *pkt, struct sockaddr_in *src) {
    int pid = pkt->player_id;
    if (pid < 0 || pid >= MAX_PLAYERS || !players[pid].is_active) return;
    players[pid].x        = pkt->x;
    players[pid].y        = pkt->y;
    players[pid].udp_addr = *src;
    printf("Move: %d to (%.1f, %.1f)\n", pid, pkt->x, pkt->y);
    broadcast_udp(udp_sock, pkt);
}
