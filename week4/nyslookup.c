#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>

int main() {
    const char *hostname = "www.naver.com";
    struct hostent *host_info;

    host_info = gethostbyname(hostname);
    if(host_info == NULL) {
        fprintf(stderr, "Error: Unable to reslove hostname %s\n", hostname);
        return 1;
    }

    printf("Hostname : %s\n", host_info->h_name);
    printf("IP Address : %s\n", inet_ntoa(*(struct in_addr *)host_info->h_addr_list[0]));

    if(host_info->h_aliases[0] != NULL) {
        printf("Aliases : \n");
        for(int i=0;host_info->h_aliases[i];i++){
            printf(" %s\n", host_info->h_aliases[i]);
        }
    }
    else {
        printf("No aliases found.\n");
    }

    return 0;
}

//VER.getaddrinfo
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <arpa/inet.h>
// #include <netdb.h>

// int main() {
//     const char *hostname = "www.naver.com";
//     struct addrinfo hints, *res, *p;

//     // 1. hints 초기화
//     memset(&hints, 0, sizeof(hints));
//     hints.ai_family = AF_INET;        // IPv4만
//     hints.ai_socktype = SOCK_STREAM;  // TCP

//     // 2. DNS 조회
//     int status = getaddrinfo(hostname, NULL, &hints, &res);
//     if (status != 0) {
//         fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
//         return 1;
//     }

//     printf("Hostname: %s\n", hostname);

//     // 3. 결과 순회 (중요)
//     for (p = res; p != NULL; p = p->ai_next) {
//         struct sockaddr_in *addr = (struct sockaddr_in *)p->ai_addr;
//         char ip_str[INET_ADDRSTRLEN];

//         // 4. IP 문자열 변환
//         inet_ntop(AF_INET, &(addr->sin_addr), ip_str, sizeof(ip_str));

//         printf("IP Address: %s\n", ip_str);
//     }

//     // 5. 메모리 해제
//     freeaddrinfo(res);

//     return 0;
// }