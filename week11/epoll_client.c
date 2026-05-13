#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

int main() {
    int client_socket = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(9090);

    connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address));

    int epfd = epoll_create(50);
    struct epoll_event event, ep_events[5];

    event.events = EPOLLIN;
    event.data.fd = 0; // 표준 입력(stdin)
    epoll_ctl(epfd, EPOLL_CTL_ADD, 0, &event);

    event.events = EPOLLIN;
    event.data.fd = client_socket; // 서버 통신용 소켓
    epoll_ctl(epfd, EPOLL_CTL_ADD, client_socket, &event);

    while (1) {
        int event_count = epoll_wait(epfd, ep_events, 5, -1); // 이벤트 발생 대기

        for (int i = 0; i < event_count; i++) {
            if (ep_events[i].data.fd == 0) { // 키보드 입력
                char message_buffer[1024];
                int read_bytes = read(0, message_buffer, 1024);
                write(client_socket, message_buffer, read_bytes);
            } else if (ep_events[i].data.fd == client_socket) { // 서버 메시지 수신
                char message_buffer[1024];
                int read_bytes = read(client_socket, message_buffer, 1024);
                if (read_bytes == 0) { // 서버 연결 끊김
                    close(client_socket);
                    return 0;
                }
                write(1, message_buffer, read_bytes); // 표준 출력
            }
        }
    }
    return 0;
}
