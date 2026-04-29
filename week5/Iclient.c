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

	while (1) {
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

		input[strcspn(input, "\n")] = '\0';

		if (strcmp(input, "quit") == 0) {
			if (send_line(sock, input) == -1) {
				log_msg("Failed to send command");
			}
			log_msg("Quitting...");
			break;
		}

		if (strcmp(input, "list") == 0) {
			if (send_line(sock, input) == -1) {
				log_msg("Failed to send command");
				break;
			}

			while (recv_line(sock, buf, sizeof(buf)) > 0) {
				printf("%s\n", buf);
				if (strcmp(buf, "END ==========") == 0) break;
			}
		}
		else if (strncmp(input, "get ", 4) == 0) {
			if (send_line(sock, input) == -1) {
				log_msg("Failed to send command");
				break;
			}

			char filename[BUF_SIZE];
			snprintf(filename, sizeof(filename), "downloaded_%s", input + 4);

			FILE *fp = fopen(filename, "wb");
			if (!fp) {
				log_msg("Failed to open file for writing");
				continue;
			}

			while (recv_line(sock, buf, sizeof(buf)) > 0) {
				if (strcmp(buf, "START") == 0) continue;
				if (strcmp(buf, "END ==========") == 0) break;
				fprintf(fp, "%s\n", buf);
			}

			fclose(fp);
			log_msg("File downloaded: %s", filename);
		}
		else if (strncmp(input, "load ", 5) == 0) {
			char *filename = input + 5;

			// 먼저 로컬 파일 존재 확인
			FILE *fp = fopen(filename, "rb");
			if (!fp) {
				log_msg("File not found: %s", filename);
				continue;
			}

			// 파일이 있을 때만 서버에 명령 전송
			if (send_line(sock, input) == -1) {
				log_msg("Failed to send command");
				fclose(fp);
				break;
			}

			// 서버 READY 받기
			if (recv_line(sock, buf, sizeof(buf)) <= 0) {
				log_msg("Failed to receive READY from server");
				fclose(fp);
				break;
			}

			if (strcmp(buf, "READY") != 0) {
				log_msg("Server is not ready: %s", buf);
				fclose(fp);
				continue;
			}

			// 파일 크기 구하기
			if (fseek(fp, 0, SEEK_END) != 0) {
				log_msg("fseek failed");
				fclose(fp);
				continue;
			}

			long file_size = ftell(fp);
			if (file_size < 0) {
				log_msg("ftell failed");
				fclose(fp);
				continue;
			}

			rewind(fp);

			char size_buf[128];
			snprintf(size_buf, sizeof(size_buf), "%ld", file_size);

			if (send_line(sock, size_buf) == -1) {
				log_msg("Failed to send file size");
				fclose(fp);
				break;
			}

			char file_buf[BUF_SIZE];
			size_t n;
			long total_sent = 0;

			while ((n = fread(file_buf, 1, sizeof(file_buf), fp)) > 0) {
				int sent = send(sock, file_buf, n, 0);
				if (sent == -1) {
					log_msg("Failed to send file data");
					fclose(fp);
					close(sock);
					return 1;
				}
				total_sent += sent;
			}

			fclose(fp);
			log_msg("Upload finished locally: %s (%ld bytes sent)", filename, total_sent);

			if (recv_line(sock, buf, sizeof(buf)) > 0) {
				printf("%s\n", buf);
			}
		}
		else {
			if (send_line(sock, input) == -1) {
				log_msg("Failed to send command");
				break;
			}

			if (recv_line(sock, buf, sizeof(buf)) > 0) {
				printf("%s\n", buf);
			}
		}
	}

	close(sock);
	return 0;
}
