#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib") // 윈속 라이브러리 링크 (MSVC 환경)

#define PORT 5000
#define BUFFER_SIZE 1024

// 윈도우용 초간단 UDP 서버
int main() {
    WSADATA wsa;
    SOCKET sockfd;
    struct sockaddr_in servaddr, cliaddr;
    int len, n;
    char buffer[BUFFER_SIZE];

    // 1. 윈속 초기화 (윈도우 필수 단계) : (하는 이유?)
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup 실패\n");
        return 1;
    }

    // 2. UDP 소켓 생성
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == INVALID_SOCKET) {
        printf("소켓 생성 실패\n");
        return 1;
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // 3. 바인드
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
        printf("바인드 실패\n");
        return 1;
    }

    printf("윈도우 UDP 서버가 %d 포트에서 대기 중...\n", PORT);

    while (1) {
        len = sizeof(cliaddr);
        // 4. 메시지 수신
        n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cliaddr, &len);
        if (n == SOCKET_ERROR) break;
        
        buffer[n] = '\0';
        printf("클라이언트: %s\n", buffer);

        // 5. 에코 메시지 전송
        sendto(sockfd, buffer, n, 0, (struct sockaddr *)&cliaddr, len);
    }

    closesocket(sockfd);
    WSACleanup(); // 윈속 종료
    return 0;
}