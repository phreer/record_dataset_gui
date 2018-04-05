#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QTimer>
#include "opencv2/opencv.hpp"
#include "opencv/highgui.h"
#include "opencv/cv.h"
#include <QPainter>
#include "tcp_reciever.h"
#include "tcp_controller.h"
#include "pxcsensemanager.h"
#include "myodatacollector.cpp"
#include "time.h"
#include "camera_capture.h"
#include "realsense_capture.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    bool connected = false;
    bool recieveServOn = false;

    bool useCamera = true;
    bool useWear = false;
    bool useMyo = false;
    bool useRealsense = true;


    Ui::MainWindow *ui;
    QImage *image;
    QImage *label2Image;

    SOCKET serv_sock=NULL;
    PXCSenseManager *pxcSenseManager;
    struct sockaddr_in serv_addr;

    myo::Hub *hub;
    myo::Myo* a_myo;
    DataCollector collector;

    camera_capture *camera_thread;
    realsense_capture *realsense_thread;
    tcp_reciever reciever_thread;

private slots:
    void startCamera();
    void startRealsense();
    void startWear();
    void startMyo();

    void readFrame();

    void stopCamera();
    void stopWearandRecv();
    void stopRealsense();

    bool connect2Wear();
    void disconnect2Wear();

    void updateUIlabel1(const QImage &image);
    void updateUIlabel2(const QImage &image);

};

#endif // MAINWINDOW_H
