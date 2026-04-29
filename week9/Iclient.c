#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5000
#define BUFFER_SIZE 1024

// 리눅스용 초간단 UDP 클라이언트
int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in servaddr;

    // 1. UDP 소켓 생성
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("소켓 생성 실패");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT); // 서버 포트 5000
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 서버 주소 (로컬)

    printf("메시지를 입력하세요 (종료: Ctrl+C)\n");

    while (1) {
        printf("입력: ");
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) break;
        buffer[strcspn(buffer, "\n")] = 0; // 엔터 문자 제거

        // 2. 서버로 메시지 전송
        sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));

        // 3. 서버로부터 응답 수신
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);   
        buffer[n] = '\0';
        printf("서버로부터의 응답: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}
