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
#include <QString>

#include "tcp_reciever.h"
#include "utils.h"
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
    void startCamera();
    void startRealsense();
    void startWear();
    void startMyo();

    void initAll(int volID, int restStateID, int gestureID);
    void startAll();
    void stopAll();

    void recordVol(int id);
    void recordUnit(int volID, int restStateID, int gestureID);

    void stopCamera();
    void stopWearandRecv();
    void stopRealsense();
    void stopMyo();

    bool connect2Wear();
    void disconnect2Wear();

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
    QTimer timer;

    SOCKET serv_sock=NULL;
    PXCSenseManager *pxcSenseManager;
    struct sockaddr_in serv_addr;

    myo::Hub *hub;
    myo::Myo* a_myo;
    DataCollector collector;

    camera_capture *camera1Thread, *camera2Thread;
    realsense_capture *realsenseThread;
    myothread *myoThread;
    tcp_reciever recieverThread;

private slots:
    // used for updating videos
    void updateCamera1(const QImage &image);
    void updateRealsense(const QImage &image);
    void updateFPSCamera1(const double fps);

    // triggered by button
    void startRecordProgress();
    void stopRecordProgress();

protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // MAINWINDOW_H
