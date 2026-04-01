#include "Icommon.h"

void cmd_list(int sock);
void cmd_get(int sock, const char *filename);
void cmd_uplode(int sock, const char *filename);
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

if (listen(server_sock, 5) == -1)
error_handling("listen() failed");

log_msg("Server started on port %d", PORT);

while (1) {
struct sockaddr_in client_addr;
socklen_t client_addr_size = sizeof(client_addr);

int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_size);
if (client_sock == -1) {
log_msg("accept() failed");
continue;
        }

log_msg("Client connected: %s:%d",
inet_ntoa(client_addr.sin_addr),
ntohs(client_addr.sin_port));

handle_client(client_sock);
close(client_sock);

log_msg("Client disconnected: %s:%d",
inet_ntoa(client_addr.sin_addr),
ntohs(client_addr.sin_port));
    }

close(server_sock);
return 0;
}

void handle_client(int sock) {
char buf[BUF_SIZE];

send_command_list(sock);

while (recv_line(sock, buf, sizeof(buf)) > 0) {
if (strcmp(buf, "list") == 0) {
cmd_list(sock);
send_command_list(sock);
        }
else if (strncmp(buf, "get ", 4) == 0) {
cmd_get(sock, buf + 4);
send_command_list(sock);
        }
else if (strncmp(buf, "load ", 5) == 0) {
cmd_uplode(sock, buf + 5);
send_command_list(sock);
        }
else if (strcmp(buf, "quit") == 0) {
break;
        }
else {
send_line(sock, "Unknown command");
send_command_list(sock);
        }
    }
}

void cmd_list(int sock) {
DIR *dir = opendir("./Files");
if (!dir) {
send_line(sock, "Failed to open directory");
return;
    }

send_line(sock, "Files:");

struct dirent *entry;
while ((entry = readdir(dir)) != NULL) {
if (entry->d_type == DT_REG) {
send_line(sock, entry->d_name);
        }
    }

closedir(dir);
send_line(sock, "END ==========");
}

void cmd_get(int sock, const char *filename) {
char filepath[BUF_SIZE];
snprintf(filepath, sizeof(filepath), "./Files/%s", filename);

FILE *fp = fopen(filepath, "rb");
if (!fp) {
send_line(sock, "File not found");
return;
    }

send_line(sock, "START");

char buf[BUF_SIZE];
while (fgets(buf, sizeof(buf), fp) != NULL) {
buf[strcspn(buf, "\n")] = '\0';
send_line(sock, buf);
    }

fclose(fp);
send_line(sock, "END ==========");
}

void cmd_uplode(int sock, const char *filename) {
char filepath[BUF_SIZE];
snprintf(filepath, sizeof(filepath), "./Files/%s", filename);

FILE *fp = fopen(filepath, "wb");
if (!fp) {
send_line(sock, "FAILED");
return;
    }

if (send_line(sock, "READY") == -1) {
fclose(fp);
return;
    }

char size_buf[128];
if (recv_line(sock, size_buf, sizeof(size_buf)) <= 0) {
fclose(fp);
return;
    }

long file_size = atol(size_buf);
if (file_size <= 0) {
fclose(fp);
send_line(sock, "FAILED");
return;
    }

log_msg("Start upload: %s (%ld bytes)", filename, file_size);

char buf[BUF_SIZE];
long total_received = 0;

while (total_received < file_size) {
int remain = (int)(file_size - total_received);
int to_read = (remain < BUF_SIZE) ? remain : BUF_SIZE;

int recv_len = recv(sock, buf, to_read, 0);
if (recv_len <= 0) {
log_msg("Upload failed while receiving data");
fclose(fp);
return;
        }

fwrite(buf, 1, recv_len, fp);
total_received += recv_len;

double percent = (total_received * 100.0) / file_size;
printf("[UPLOAD] %s : %.2f%% (%ld/%ld bytes)\n",
filename, percent, total_received, file_size);
fflush(stdout);
    }

fclose(fp);
send_line(sock, "UPLOAD DONE");
log_msg("Upload complete: %s", filename);
}