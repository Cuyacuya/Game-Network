#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 5000
#define BUFFER_SIZE 1024

// 윈도우용 초간단 UDP 클라이언트
int main() {
    WSADATA wsa;
    SOCKET sockfd;
    struct sockaddr_in servaddr;
    char buffer[BUFFER_SIZE];
    int n;

    // 1. 윈속 초기화
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // 2. 소켓 생성
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf("메시지를 입력하세요 (종료: Ctrl+C)\n");

    while (1) {
        printf("입력: ");
        if (gets(buffer) == NULL) break; // 초간단 구현을 위해 gets 사용

        // 3. 전송
        sendto(sockfd, buffer, (int)strlen(buffer), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

        // 4. 수신
        n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);
        if (n != SOCKET_ERROR) {
            buffer[n] = '\0';
            printf("서버 응답: %s\n", buffer);
        }
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}
    