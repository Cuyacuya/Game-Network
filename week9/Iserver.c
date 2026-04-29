#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5000
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;

    // 1. UDP 소켓 생성 (AF_INET: IPv4, SOCK_DGRAM: UDP)
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { 
        perror("소켓 생성 실패");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET; // IPv4 주소 체계
    servaddr.sin_addr.s_addr = INADDR_ANY; // 모든 인터페이스로부터 수신
    servaddr.sin_port = htons(PORT); // 포트 번호 5000 설정 (Network Byte Order로 변환)

    // 2. 소켓에 주소와 포트 할당
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("바인드 실패");
        exit(EXIT_FAILURE);
    }

    printf("리눅스 UDP 서버가 %d 포트에서 대기 중...\n", PORT);

    while (1) {
        len = sizeof(cliaddr);
        // 3. 클라이언트로부터 메시지 수신
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cliaddr, &len);
        buffer[n] = '\0';

        

        printf("클라이언트 메시지: %s\n", buffer);

        // 4. 클라이언트에게 받은 메시지를 그대로 다시 전송 (에코)
        sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&cliaddr, len);
    }

    close(sockfd);
    return 0;
}
