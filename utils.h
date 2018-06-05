#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <string.h>
#include <Windows.h>
#include <io.h>
#include <time.h>

#define SERV_IP "192.168.1.199"
#define RECV_IP "192.168.1.195"
#define SERV_PORT 1893

#define MAX_CMD_LEN 32
#define BUFFER_SIZE 4096
#define SLEEP_TIME 2000
#define RECV_PORT 9988
#define FILENAME_SIZE 128
#define COMMAND_LEN 5

static char send_buffer[BUFFER_SIZE], recv_buffer[BUFFER_SIZE];

enum command_t { hello_c, start_c, stop_c, send_c, bye_c };
static const char* COMMAND[] = { "HELLO", "START", "STOP0", "SEND0", "BYE00" };

// send n bytes in buffer to a given socket
int sendn(SOCKET sock, void * buffer_ptr, size_t len);
// util functions for setting socket address
void set_addr_s(struct sockaddr_in *addr, const char *ip, u_short port);
bool send_command(SOCKET scoket, command_t c);
int64_t ntoh64(void *ptr);
#endif // UTILS_H
