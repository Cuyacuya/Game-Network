#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>

int main() {
    //1.TCP 서버 소켓 생성
    int server_socket = socket(PF_INET, SOCK_STEAM, 0);
    struct sockaddr_in server_address;

    //2. 서버 주소 설정
    memset(&server_address, 0 ,sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(9090);

    //3. bind() : 서버 소켓에 IP주소와 포트 번호 할당
    bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    //4. listen()
    listen(server_socket, 5);

    //5. fd_set 생성
    //master_fds : fd목록 유지 원본
    //temp_fds : select()에 넘길 임시 복사본 <- 여기서 select()이 변경
    fd_set master_fds, temp_fds;
    FD_ZERO(&master_fds);
    //6. 서버 소켓을 fd_set에 등록
    FD_SET(server_address, &master_fds);
    int max_fd = server_socket;

    
    while(1){
        temp_fds = master_fds;
        //7. select()로 이벤트 대기
        //select(검사할 fd범위, 읽기 이벤트 감시, 쓰기 이벤트 감시 안함, 예외 이벤트 감시 안함, 이벤트 발생까지 무한 대기)
        select(max_fd + 1, &temp_fds, 0,0,0);
        
        //서버 이벤트 처리 흐름
        for(int cur_fd=0;cuf_fd<=max_fd;cur_fd++){
            if(FD_ISSET(cur_fd, &temp_fds)){
                //8. 서버 소켓에 이벤트 발생 -> accept()
                if(cur_fd == server_socket){
                    //새로운 클라이언트 접속
                    int client_socket = accept(server_socket, NULL,NULL);
                    FD_SET(client_socket, &master_fds);
                    if(max_fd < client_socket) max_fd = client_socket;
                }
                else { //9. 클라이언트 소켓에 이벤트 발생 -> read()
                    char message_buff[1024];
                    int read_bytes = read(cur_fd, message_buffer, 1024);

                    if(read_bytes == 0) {
                        //연결 종료
                        FD_CLR(cur_fd, &master_fds);
                        close(cur_fd);
                    }
                    else {
                        //10. 받은 메시지를 다른 클라에게 write()
                        //브로드캐스트
                        for(int target_fd = 0;target_fd <= max_fd; target_fd++){
                            if(FD_ISSET(target_fd, &master_fds) && target_fd != server_socket && target_fd != cur_fd){
                                write(target_fd, message_buff, read_bytes);
                            }
                        }
                    }
                }
            }
        }
    }

}