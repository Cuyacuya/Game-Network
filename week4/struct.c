#include <stdio.h>
#include <string.h>
#include <arpa/inet.h> //c에서 네트워크 주소 변환 함수 사용

int main(){
    struct sockaddr_in server_addr;
    
    memset(&server_addr, 0, sizeof(server_addr)); //구조체를 0으로 초기화(쓰레기값 제거)

    server_addr.sin_family = AF_INET; //IPv4 주소 체계 사용
    server_addr.sin_port = htons(8080); //포트 번호 설정

    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr); //ip -> 이진
    printf("server IP : %s\n", inet_ntoa(server_addr.sin_addr));
    printf("Server Port : %d\n", ntohs(server_addr.sin_port));

    printf("port value in memory : 0x%04X\n", server_addr.sin_port);
    printf("real port value : 0x%04X\n", ntohs(server_addr.sin_port));
    return 0;
}