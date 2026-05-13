/********************************************************************************
 * 파일명: step1_get_buf.c
 ********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

void error_handling(const char *msg) {
    perror(msg);
    exit(1);
}

int main() {
    int sock;
    int snd_buf, rcv_buf;
    socklen_t optlen;

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    optlen = sizeof(snd_buf);
    if (getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (void*)&snd_buf, &optlen) == -1)
        error_handling("getsockopt() error");

    optlen = sizeof(rcv_buf);
    if (getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (void*)&rcv_buf, &optlen) == -1)
        error_handling("getsockopt() error");

    printf("[기본 소켓 버퍼 크기]\n");
    printf("송신 버퍼: %d bytes\n", snd_buf);
    printf("수신 버퍼: %d bytes\n", rcv_buf);

    close(sock);
    return 0;
}