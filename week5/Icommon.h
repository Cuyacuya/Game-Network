#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <dirent.h>

#define PORT 5000
#define BUF_SIZE 4096

void error_handling(const char *message) {
    fprintf(stderr, "[ERROR] %s: %s(%d)\n", message, strerror(errno), errno);
    exit(EXIT_FAILURE);
}

void log_msg(const char *format, ...) {
    va_list args;
    fprintf(stdout, "[LOG] ");
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
    fprintf(stdout, "\n");
    fflush(stdout);
}

int send_line(int sock, const char *line) {
    char buf[BUF_SIZE];
    int len = snprintf(buf, sizeof(buf), "%s\n", line);
    return send(sock, buf, len, 0);
}

int recv_line(int sock, char *buf, size_t size) {
    int i = 0;
    char c;

    while (i < (int)size - 1) {
        int recv_len = recv(sock, &c, 1, 0);
        if (recv_len <= 0) {
            buf[i] = '\0';
            return -1;
        }
        if (c == '\n') break;
        buf[i++] = c;
    }

    buf[i] = '\0';
    return i;
}

void send_command_list(int sock) {
    send_line(sock, "COMMANDS: list, get <filename>, load <filename>, quit");
}