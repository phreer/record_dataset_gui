#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QTimer>
#include "opencv2/opencv.hpp"
#include "opencv/highgui.h"
#include "opencv/cv.h"
#include <QPainter>
#include <QCloseEvent>

#include "tcp_reciever.h"
#include "tcp_controller.h"
#include "pxcsensemanager.h"
#include "time.h"
#include "camera_capture.h"
#include "realsense_capture.h"
#include "myothread.h"

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
    bool useWear = false;
    bool useMyo = true;
    bool useRealsense = false;


    Ui::MainWindow *ui;
    QImage *image;
    QImage *label2Image;
    QTimer timer;

    SOCKET serv_sock=NULL;
    PXCSenseManager *pxcSenseManager;
    struct sockaddr_in serv_addr;

    myo::Hub *hub;
    myo::Myo* a_myo;
    DataCollector collector;

    camera_capture *camera_thread;
    realsense_capture *realsense_thread;
    myothread *myo_thread;
    tcp_reciever reciever_thread;

private slots:
    void startCamera();
    void startRealsense();
    void startWear();
    void startMyo();

    void stopCamera();
    void stopWearandRecv();
    void stopRealsense();
    void stopMyo();

    bool connect2Wear();
    void disconnect2Wear();

    void updateUIlabel1(const QImage &image);
    void updateUIlabel2(const QImage &image);

protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // MAINWINDOW_H
