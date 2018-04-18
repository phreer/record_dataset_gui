#ifndef TCP_RECIEVER_H
#define TCP_RECIEVER_H
#include "tcp_controller.h"
#include <QObject>
#include <QThread>

class tcp_reciever : public QThread
{
public:
    tcp_reciever();
    void stop();

private:
    SOCKET recv_sock=NULL;
    SOCKET data_sock=NULL;
    bool end;

    void run();
};

#endif // TCP_RECIEVER_H
