#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>

int main() {
    int client_socket = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(9090);

    connect(client_socket, (struct sockaddr_in*)&server_address, sizeof(server_address));

    fd_set master_fds, temp_fds;
    FD_ZERO(&master_fds);
    FD_SET(0,&master_fds);
    FD_SET(client_socket, &master_fds);
    int max_fd = client_socket;

    while(1){
        temp_fds = master_fds;
        select(max_fd + 1, &temp_fds,0,0,0);

        //키보드 입력 받을 시 : 0
        if(FD_ISSET(0, &temp_fds)){
            char msg_buf[1024];
            int read_bytes = read(0, msg_buf, 1024);
            write(client_socket, msg_buf,read_bytes);
        }

        //서버로부터 메시지를 수신한 경우
        if (FD_ISSET(client_socket, &temp_fds)) {
            char message_buffer[1024];
            int read_bytes = read(client_socket, message_buffer, 1024);
            if (read_bytes == 0) break;
            write(1, message_buffer, read_bytes);
        }
    }
    close(client_socket);
    return 0;
}