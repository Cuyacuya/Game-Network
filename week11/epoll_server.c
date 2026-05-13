#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

int main() {
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(9090);

    bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    listen(server_socket, 5);

    int epfd = epoll_create(50);
    struct epoll_event event, ep_events[50];
    
    event.events = EPOLLIN;
    event.data.fd = server_socket;
    epoll_ctl(epfd, EPOLL_CTL_ADD, server_socket, &event); // 서버 소켓 epoll에 등록

    int client_sockets[100]; // 연결된 클라이언트 소켓들을 저장할 배열
    int client_count = 0;

    while (1) {
        int event_count = epoll_wait(epfd, ep_events, 50, -1); // 이벤트 발생 대기

        for (int i = 0; i < event_count; i++) {
            int current_fd = ep_events[i].data.fd;

            if (current_fd == server_socket) {
                // 새로운 클라이언트 접속
                int client_socket = accept(server_socket, NULL, NULL);
                event.events = EPOLLIN;
                event.data.fd = client_socket;
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_socket, &event);
                client_sockets[client_count++] = client_socket; // 배열에 추가
            } else {
                // 클라이언트로부터 메시지 수신
                char message_buffer[1024];
                int read_bytes = read(current_fd, message_buffer, 1024);

                if (read_bytes == 0) {
                    // 연결 종료: epoll에서 자동 제거되므로 배열에서만 삭제 후 닫기
                    epoll_ctl(epfd, EPOLL_CTL_DEL, current_fd, NULL);
                    close(current_fd);
                    for (int j = 0; j < client_count; j++) {
                        if (client_sockets[j] == current_fd) {
                            client_sockets[j] = client_sockets[--client_count];
                            break;
                        }
                    }
                } else {
                    // 다른 모든 클라이언트에게 메시지 브로드캐스트
                    for (int j = 0; j < client_count; j++) {
                        if (client_sockets[j] != current_fd) {
                            write(client_sockets[j], message_buffer, read_bytes);
                        }
                    }
                }
            }
        }
    }
    return 0;
}
