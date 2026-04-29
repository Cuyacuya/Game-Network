#include <stdio.h>
#include <string.h>
#include <arpa/inet.h> //c에서 네트워크 주소 변환 함수 사용

int main() {
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5000);

    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr); //prsentation to network
    printf("Server IP : %s\n", inet_ntoa(server_addr.sin_addr));
    printf("Server Port : %d\n", ntohs(server_addr.sin_port));

    printf("Port value in memory : 0x%04X\n", server_addr.sin_port);
    printf("real port value : 0x%04X\n", ntohs(server_addr.sin_port));
}