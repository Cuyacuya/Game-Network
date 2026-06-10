#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#pragma comment(lib, "ws2_32.lib")

#define MAX_PLAYERS 1000
#define PORT        8000

#define OP_TCP_RECV 1
#define OP_UDP_RECV 2

typedef struct {
    float x, y;
    int hp;
    int is_active;
    SOCKET tcp_sock;
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

typedef struct {
    OVERLAPPED overlapped; //iocp 식별용
    WSABUF wsabuf;
    char buffer[sizeof(Packet)];
    int op_type;
    SOCKET socket;
    int player_id;
    struct sockaddr_in addr;
    INT addr_len;
} IOContext;

typedef struct {
    SOCKET udp_sock;
    HANDLE iocp;
} ThreadParam;

Player players[MAX_PLAYERS];

int          alloc_player(SOCKET sock);
void         free_player(SOCKET sock);
void         broadcast_tcp(Packet *pkt);
void         broadcast_udp(SOCKET udp_sock, Packet *pkt);
void         on_tcp(SOCKET udp_sock, Packet *pkt, IOContext *ctx);
void         on_udp(SOCKET udp_sock, Packet *pkt, struct sockaddr_in *src);
void         post_udp_recv(SOCKET udp_sock, IOContext *ctx);
void         post_tcp_recv(IOContext *ctx);
DWORD WINAPI worker_thread(LPVOID param);

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    for (int i = 0; i < MAX_PLAYERS; i++) {
        players[i].is_active = 0;
        players[i].tcp_sock  = INVALID_SOCKET;
    }

    HANDLE iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

    SOCKET tcp_listen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    SOCKET udp_sock   = WSASocket(AF_INET, SOCK_DGRAM,  0, NULL, 0, WSA_FLAG_OVERLAPPED);

    struct sockaddr_in addr = {0};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(PORT);

    bind(tcp_listen, (struct sockaddr *)&addr, sizeof(addr));
    listen(tcp_listen, 5);
    bind(udp_sock, (struct sockaddr *)&addr, sizeof(addr));

    CreateIoCompletionPort((HANDLE)udp_sock, iocp, 0, 0);

    IOContext *uctx = calloc(1, sizeof(IOContext));
    uctx->player_id = -1;
    post_udp_recv(udp_sock, uctx);

    ThreadParam *tp = malloc(sizeof(ThreadParam));
    tp->udp_sock = udp_sock;
    tp->iocp     = iocp;
    CreateThread(NULL, 0, worker_thread, tp, 0, NULL);

    printf("Server started. Port: %d\n", PORT);

    while (1) {
        SOCKET client = accept(tcp_listen, NULL, NULL);
        CreateIoCompletionPort((HANDLE)client, iocp, 0, 0);
        IOContext *ctx = calloc(1, sizeof(IOContext));
        ctx->socket    = client;
        ctx->player_id = -1;
        post_tcp_recv(ctx);
    }
    return 0;
}

int alloc_player(SOCKET sock) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!players[i].is_active) {
            players[i].is_active = 1;
            players[i].hp        = 100;
            players[i].tcp_sock  = sock;
            return i;
        }
    }
    return -1;
}

void free_player(SOCKET sock) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (players[i].tcp_sock == sock) {
            players[i].is_active = 0;
            players[i].tcp_sock  = INVALID_SOCKET;
            printf("Logout (disconnect): %d\n", i);
            return;
        }
    }
}

void broadcast_tcp(Packet *pkt) {
    for (int i = 0; i < MAX_PLAYERS; i++)
        if (players[i].is_active && players[i].tcp_sock != INVALID_SOCKET)
            send(players[i].tcp_sock, (char *)pkt, sizeof(Packet), 0);
}

void broadcast_udp(SOCKET udp_sock, Packet *pkt) {
    for (int i = 0; i < MAX_PLAYERS; i++)
        if (players[i].is_active && players[i].udp_addr.sin_family == AF_INET)
            sendto(udp_sock, (char *)pkt, sizeof(Packet), 0,
                   (struct sockaddr *)&players[i].udp_addr, sizeof(struct sockaddr_in));
}

void on_tcp(SOCKET udp_sock, Packet *pkt, IOContext *ctx) {
    int pid = pkt->player_id;
    switch (pkt->type) {
        case 1: { // 로그인: 서버가 ID 부여 후 응답
            pid = alloc_player(ctx->socket);
            if (pid < 0) return;
            ctx->player_id = pid;
            strncpy(players[pid].username, pkt->username, 32);
            Packet resp    = *pkt;
            resp.player_id = pid;
            send(ctx->socket, (char *)&resp, sizeof(resp), 0);
            printf("Login: %d (%s)\n", pid, players[pid].username);
            broadcast_tcp(&resp);
            break;
        }
        case 2: // 로그아웃
            if (pid < 0 || pid >= MAX_PLAYERS) return;
            players[pid].is_active = 0;
            players[pid].tcp_sock  = INVALID_SOCKET;
            printf("Logout: %d\n", pid);
            broadcast_tcp(pkt);
            ctx->player_id = -1;
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

void on_udp(SOCKET udp_sock, Packet *pkt, struct sockaddr_in *src) {
    int pid = pkt->player_id;
    if (pid < 0 || pid >= MAX_PLAYERS || !players[pid].is_active) return;
    players[pid].x        = pkt->x;
    players[pid].y        = pkt->y;
    players[pid].udp_addr = *src;
    printf("Move: %d to (%.1f, %.1f)\n", pid, pkt->x, pkt->y);
    broadcast_udp(udp_sock, pkt);
}

void post_udp_recv(SOCKET udp_sock, IOContext *ctx) {
    DWORD flags = 0, bytes = 0;
    ctx->op_type     = OP_UDP_RECV;
    ctx->wsabuf.buf  = ctx->buffer;
    ctx->wsabuf.len  = sizeof(Packet);
    ctx->addr_len    = sizeof(ctx->addr);
    memset(&ctx->overlapped, 0, sizeof(OVERLAPPED));
    WSARecvFrom(udp_sock, &ctx->wsabuf, 1, &bytes, &flags,
                (struct sockaddr *)&ctx->addr, &ctx->addr_len, &ctx->overlapped, NULL);
}

void post_tcp_recv(IOContext *ctx) {
    DWORD flags = 0, bytes = 0;
    ctx->op_type    = OP_TCP_RECV;
    ctx->wsabuf.buf = ctx->buffer;
    ctx->wsabuf.len = sizeof(Packet);
    memset(&ctx->overlapped, 0, sizeof(OVERLAPPED));
    WSARecv(ctx->socket, &ctx->wsabuf, 1, &bytes, &flags, &ctx->overlapped, NULL);
}

DWORD WINAPI worker_thread(LPVOID param) {
    ThreadParam *tp   = (ThreadParam *)param;
    SOCKET udp_sock   = tp->udp_sock;
    HANDLE iocp       = tp->iocp;
    while (1) {
        DWORD     bytes;
        ULONG_PTR key;
        IOContext *ctx;
        GetQueuedCompletionStatus(iocp, &bytes, &key, (OVERLAPPED **)&ctx, INFINITE);
        if (!ctx) continue;

        if (ctx->op_type == OP_UDP_RECV) {
            if (bytes > 0)
                on_udp(udp_sock, (Packet *)ctx->buffer, &ctx->addr);
            post_udp_recv(udp_sock, ctx);
        } else {
            if (bytes == 0) {
                free_player(ctx->socket);
                closesocket(ctx->socket);
                free(ctx);
            } else {
                on_tcp(udp_sock, (Packet *)ctx->buffer, ctx);
                post_tcp_recv(ctx);
            }
        }
    }
    return 0;
}
