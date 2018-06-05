#ifndef TCP_RECIEVER_H
#define TCP_RECIEVER_H
#include <QObject>
#include <QThread>
#include "utils.h"

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
