#ifndef MYOTHREAD_H
#define MYOTHREAD_H
#include <QThread>
#include <iostream>
#include <QTime>
#include <cstring>
#include "myodatacollector.cpp"

#define MAX_FILENAME 512

class myothread : public QThread{
    Q_OBJECT
public:
    myothread(const char filename[]);
    void stop();

protected:
    void run() override;

private:
    void initMyo();
    std::ofstream ofile;
    char ofilename[MAX_FILENAME];
    bool end = false;
    myo::Hub *hub;
    myo::Myo *a_myo;
    DataCollector collector;
    const double sample_rate = 60.0;
};

#endif // MYOTHREAD_H
