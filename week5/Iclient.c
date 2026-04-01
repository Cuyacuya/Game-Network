#include "Icommon.h"

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) error_handling("socket() failed");

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        error_handling("connect() failed");

    char buf[BUF_SIZE];
    while(1) {
        if (recv_line(sock, buf, sizeof(buf)) <= 0) {
            log_msg("Connection closed by server");
            break;
        }
        printf("%s\n", buf);
        printf("> ");
        fflush(stdout);

        char input[BUF_SIZE];
        if (!fgets(input, sizeof(input), stdin)) {
            log_msg("Failed to read input");
            break;
        }
        input[strcspn(input, "\n")] = '\0'; // Remove newline

        if (send_line(sock, input) == -1) {
            log_msg("Failed to send command");
            break;
        }

        if (strcmp(input, "quit") == 0) {
            log_msg("Quitting...");
            break;
        }

        if (strcmp(input, "list") == 0) {
            while(recv_line(sock, buf, sizeof(buf)) > 0) {
                printf("%s\n", buf);
                if(strcmp(buf, "END ==========") == 0) break;
            }
        }

        if (strncmp(input, "get ", 4) == 0) {
            char filename[BUF_SIZE];
            snprintf(filename, sizeof(filename), "downloaded_%s", input + 4);
            FILE *fp = fopen(filename, "wb");
            if (!fp) {
                log_msg("Failed to open file for writing");
                continue;
            }
            while(recv_line(sock, buf, sizeof(buf)) > 0) {
                if(strcmp(buf, "END ==========") == 0) break;
                fprintf(fp, "%s\n", buf);
            }
            fclose(fp);
            log_msg("File downloaded: %s", filename);
        }
    }
    close(sock);
    return 0;
}