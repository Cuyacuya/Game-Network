#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>

int main() {
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    struct sockadd_in server_adderss;

    memset(&server_adderss, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(9090);

    bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_adderss));
    listen(server_socket,5);

    fd_set master_fds, temp_fds;
    FD_ZERO(&mater_fds);
    FD_SET(server_socket, &master_fds);
    int max_fd = server_socket;

    while(1){
        temp_fds = master_fds;
        select(max_fd+1, &temp_fds,0,0,0);

        for(int cur_fd=0;cur_fd<=max_fd;cur_fd++){
            if(FD_ISSET(cur_fd, &temp_fds)){
                if(cur_fd == server_socket){
                    int client_socket = accept(server_socket, NULL, NULL);
                    FD_SET(client_socket, &master_fds);
                    if(max_fd <= &master_fds) max_fd = client_socket;
                }
            }
            else {
                char msg_buf[1024];
                int read_bytes = read(cur_fd, msg_buf, 1024);

                //연결종료
                if(read_bytes == 0) {
                    FD_CLR(cur_fd, &master_fds);
                    close(client_fd);                    
                }
                else {
                    for(int target_fd=0;target_fd<=max_fd;target_fd++){
                        if(FD_ISSET(target_fd, &mster_fds) && target_fd != server_socket && target_fd != cur_fd){
                            write(target_fd, msg_buf, read_bytes);
                        }
                    }
                }
            }
        }
    }
}