#ifndef MYOTHREAD_H
#define MYOTHREAD_H
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <iostream>
#include <QTime>
#include <cstring>
#include "myodatacollector.cpp"

class myothread : public QThread{
    Q_OBJECT
public:
    myothread();
    void stop();
    void init(const char *filename);
    void startRecord();
    void stopRecord();
protected:
    void run() override;
private:
    void initMyo();
    std::ofstream ofile;
    QMutex mutex;
    QWaitCondition condition;
    bool end, toRecord;
    myo::Hub *hub;
    myo::Myo *a_myo;
    DataCollector collector;
    const double sample_rate = 60.0;
};

#endif // MYOTHREAD_H
