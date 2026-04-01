#include "Icommon.h"

void cmd_list(int sock);
void cmd_get(int sock, const char *filename);
void handle_client(int sock);

int main() {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) error_handling("socket() failed");

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        error_handling("bind() failed");

    if (listen(server_sock, 5) == -1) error_handling("listen() failed");

    log_msg("Server started on port %d", PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_size = sizeof(client_addr);
        int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_size);
        if (client_sock == -1) {
            log_msg("accept() failed");
            continue;
        }
        log_msg("Client connected: %s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        handle_client(client_sock);
        close(client_sock);
        log_msg("Client disconnected: %s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }
    return 0;
}

void handle_client(int sock) {
    send_command_list(sock);
    char buf[BUF_SIZE];
    while (recv_line(sock, buf, sizeof(buf)) > 0) {
        if (strcmp(buf, "list") == 0) {
            cmd_list(sock);
        } else if (strncmp(buf, "get ", 4) == 0) {
            cmd_get(sock, buf + 4);
        } else if (strcmp(buf, "quit") == 0) {
            break;
        } else {
            send_line(sock, "Unknown command");
        }
        send_command_list(sock);
    }
}

void cmd_list(int sock) {
    DIR *dir = opendir("./files");
    if (!dir) {
        send_line(sock, "Failed to open directory");
        return;
    }

    send_line(sock, "Files:");
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { // Regular file
            send_line(sock, entry->d_name);
        }
    }
    closedir(dir);
    send_line(sock, "END ==========");
}

void cmd_get(int sock, const char *filename) {
    char filepath[BUF_SIZE];
    snprintf(filepath, sizeof(filepath), "./files/%s", filename);
    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        send_line(sock, "File not found");
        return;
    }
    char buf[BUF_SIZE];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0) {
        if (send(sock, buf, n, 0) == -1) {
            log_msg("Failed to send file data");
            break;
        }
    }
    fclose(fp);
    send_line(sock, "END ==========");
}