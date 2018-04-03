/*
 * Author: Phree Liu
 * Created Date: Mar.25.2018
 * Version: 0.1.0
 * this class is a thread(derived from QThread) to run a server who recieve data
 * from smart watch
 */
#include "tcp_reciever.h"



tcp_reciever::tcp_reciever(){

}

/*
 * transform 64 byte int in network byte order to host byte order
 */
int64_t ntoh64(void *ptr){
    char *cptr = (char*) ptr;
    char tmp_buffer[8];
    for(int i=0; i<8; i++){
        tmp_buffer[7-i] = *(cptr+i);
        printf("%x", *(cptr+i));
    }
    int64_t res = *((int64_t*)tmp_buffer);
    return res;
}

void tcp_reciever::run(){
    struct sockaddr_in recv_addr, send_addr;
    int send_addr_len = sizeof(send_addr);
    memset(&recv_addr, 0, sizeof(recv_addr));
    memset(&send_addr, 0, sizeof(send_addr));
    set_addr_s(&recv_addr, RECV_IP, RECV_PORT);
    //socket for recieve data from wear
    recv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (recv_sock == INVALID_SOCKET) {
        printf("main: get recv_sock failed. %ld", WSAGetLastError());
    }

    if ((bind(recv_sock, (struct sockaddr *) &recv_addr, sizeof(recv_addr))) == SOCKET_ERROR) {
        printf("main: bind socket failed. %ld", WSAGetLastError());
    }
    if ((listen(recv_sock, 5)) == SOCKET_ERROR) {
        printf("main: listen failed. %ld", WSAGetLastError());
    }
    for(;;){
        data_sock = accept(recv_sock, (struct sockaddr *)&send_addr, &send_addr_len);
        //get the length of file
        int n_read = 0;
        int off = 0;
        int64_t n_left = 8;
        while (n_left > 0) {
            if ((n_read = recv(data_sock, recv_buffer+off, n_left, 0)) < 0) {
                printf("main: recieve length failed.\n");
                exit(-1);
            }
            n_left -= n_read;
            off += n_read;
        }


        int64_t *length = (int64_t*)recv_buffer;
        n_left = ntoh64(length);

        printf("Length: %d\n", n_left);
        char filename[FILENAME_SIZE];
        time_t ctime;
        time(&ctime);
        sprintf(filename, "%d.txt", ctime);
        FILE *file = fopen(filename, "w+");
        if (file == NULL) {
            printf("main: open file failed.\n");
            exit(-1);
        }
        //recieve file
        while (n_left > 0) {
            if ((n_read = recv(data_sock, recv_buffer, BUFFER_SIZE, 0)) < 0) {
                printf("main: recieve file failed. %ld\n", WSAGetLastError());
                exit(-1);
            }
            fwrite(recv_buffer, sizeof(char), n_read, file);
            n_left -= n_read;
        }
        fclose(file);
    }
}
