/*
 * Author: Phree Liu
 * Created Date: Mar.25.2018
 * Version: 0.1.0
 * this class is a thread(derived from QThread) to run a server who recieve data
 * from smart watch
 */
#include "tcp_reciever.h"

tcp_reciever::tcp_reciever(){
    end = false;
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
        printf("tcp_reciever: get recv_sock failed. %ld\n", WSAGetLastError());
    }

    if ((bind(recv_sock, (struct sockaddr *) &recv_addr, sizeof(recv_addr))) == SOCKET_ERROR) {
        printf("tcp_reciever: bind socket failed. %ld\n", WSAGetLastError());
    }
    if ((listen(recv_sock, 5)) == SOCKET_ERROR) {
        printf("tcp_reciever: listen failed. %ld\n", WSAGetLastError());
    }
    unsigned long flag=1;
    if(ioctlsocket(recv_sock, FIONBIO, &flag)!=0){
        printf("tcp_reciever: unable to set recv_sock non-blocking. %ld\n", WSAGetLastError());
    }
    for(;;){
        if(!end){
            data_sock = accept(recv_sock, (struct sockaddr *)&send_addr, &send_addr_len);
            if(data_sock==INVALID_SOCKET){
                if(!WSAGetLastError()==WSAEWOULDBLOCK) // no bracket
                    printf("tcp_reciever: get INVALID SOCK. %ld\n", WSAGetLastError());
                continue;
            }

            //get the length of file
            int n_read = 0;
            int off = 0;
            int64_t n_left = 8;
            while (n_left > 0) {
                if ((n_read = recv(data_sock, recv_buffer+off, n_left, 0)) < 0) {
                    if(WSAGetLastError()!=WSAEWOULDBLOCK){
                        printf("tcp_reciever: recieve length failed. %ld\n", WSAGetLastError());
                    }
                    else{
                        continue;
                    }
                }
                n_left -= n_read;
                off += n_read;
            }


            int64_t *length = (int64_t*)recv_buffer;
            n_left = ntoh64(length);

            printf("Length: %ld\n", n_left);
            char filename[FILENAME_SIZE];
            time_t ctime;
            time(&ctime);
            sprintf(filename, "%ld.txt", ctime);
            FILE *file = fopen(filename, "w+");
            if (file == NULL) {
                printf("tcp_reciever: open file failed.\n");
                exit(-1);
            }
            //recieve file
            while (n_left > 0) {
                if ((n_read = recv(data_sock, recv_buffer, BUFFER_SIZE, 0)) < 0) {
                    if(WSAGetLastError()!=WSAEWOULDBLOCK){
                        printf("tcp_reciever: recieve file failed. %ld\n", WSAGetLastError());
                    }
                    else{
                        continue;
                    }
                }
                fwrite(recv_buffer, sizeof(char), n_read, file);
                n_left -= n_read;
            }
            fclose(file);
        }else{
            printf("tcp_reciever terminated.\n");
            return;
        }
    }
}

void tcp_reciever::stop(){
    end = true;
}
