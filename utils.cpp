#include "tcp_controller.h"
int sendn(SOCKET sock, void * buffer_ptr, size_t len) {
    size_t n_left = len;
    size_t n_written = 0;
    char *ptr;
    ptr = (char*) buffer_ptr;
    while (n_left > 0) {
        n_written = send(sock, ptr, n_left, 0);
        if ((n_written) == SOCKET_ERROR) {
            printf("sendn: send error. %ld\n", WSAGetLastError());
        }
        n_left -= n_written;
        ptr += n_written;
    }
    return len - n_left;
}
void set_addr_s(struct sockaddr_in *addr, const char *ip, u_short port) {
    memset(addr, 0, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr->sin_addr);
}

bool send_command(SOCKET socket, int c) {
    strcpy_s(send_buffer, BUFFER_SIZE, COMMAND[c]);
    if (sendn(socket, send_buffer, COMMAND_LEN) != COMMAND_LEN) {
        printf("send_command: send error. %ld\n", WSAGetLastError());
        return false;
    }
    return true;
}
