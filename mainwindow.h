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

    bool useCamera = false;
    bool useWear = true;
    bool useMyo = false;
    bool useRealsense = false;

    const float frameRate = 60;
    const cv::Size size = cv::Size(640, 480);

    Ui::MainWindow *ui;
    QTimer *timer;
    QImage *image;
    QImage *label2Image;

    cv::VideoCapture capture;
    cv::VideoWriter writer; //write camera stream
    cv::VideoWriter writerColorRe, writerIrRe, writerDepthRe; //write realsense stream
    cv::Mat img;
    cv::Mat frameIR;
    cv::Mat frameColor;
    cv::Mat frameDepth;
    SOCKET serv_sock=NULL;
    PXCSenseManager *pxcSenseManager;
    struct sockaddr_in serv_addr;

    tcp_reciever reciever_thread;
    myo::Hub *hub;
    myo::Myo* a_myo;
    DataCollector collector;

private slots:
    void startCamera();
    void startRealsense();
    void startWear();
    void startTimer();
    void startMyo();

    void readFrame();

    void stopCamera();
    void stopWearandRecv();
    void stopRealsense();
    void stopTimer();

    bool connect2Wear();
    void disconnect2Wear();

    void takingPictures();

};

#endif // MAINWINDOW_H
