#include "iconmon.h"

void cmd_list(int sock);
void cmd_get(int sock, const char *filename);
void handle_client(int sock);

int main() {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(server_sock == -1) error_handling("socket() error");

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;   
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);                                                                                    
    server_addr.sin_port = htons(PORT);
        
    if(bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        error_handling("bind() error");
    if(listen(server_sock, 5) == -1)
        error_handling("listen() error");
    log("Server is listening on port %d", PORT);
    while(1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_size = sizeof(client_addr);
        int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_size);
        if(client_sock == -1) {
            log("accept() error");
            continue;
        }
        log("Client connected: %s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        handle_client(client_sock);
        close(client_sock);
        log("Client disconnected: %s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }
    close(server_sock);
    return 0;
}

void handle_client(int sock) {
    char buf[BUF_SIZE];
    while(1) {
        if(recv_line(sock, buf, sizeof(buf)) <= 0) break;
        log("Received command: %s", buf);
        if(strncmp(buf, "LIST", 4) == 0) {
            cmd_list(sock);
        } else if(strncmp(buf, "GET ", 4) == 0) {
            cmd_get(sock, buf + 4);
        } else {
            send_line(sock, "ERROR: Unknown command");
        }
    }
}

void cmd_list(int sock) {
    DIR *dir = opendir("./files");
    if(!dir) {
        send_line(sock, "ERROR: Failed to open directory");
        return;
    }
    send_line(sock, "Files:");
    struct dirent *entry;
    while((entry = readdir(dir)) != NULL) {
        if(entry->d_type == DT_REG) { // Regular file
            send_line(sock, entry->d_name);
        }
    }
    closedir(dir);
    send_line(sock, "=====END=====");
}

void cmd_get(int sock, const char *filename) {
    char filepath[BUF_SIZE];
    snprintf(filepath, sizeof(filepath), "./files/%s", filename);
    FILE *file = fopen(filepath, "r");
    if(!file) {
        send_line(sock, "ERROR: File not found");
        return;
    }
    char buf[BUF_SIZE];
    while(fgets(buf, sizeof(buf), file)) {
        send_line(sock, buf);
    }
    fclose(file);
    send_line(sock, "=====END=====");
}