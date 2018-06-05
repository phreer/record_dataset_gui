#ifndef CAMERA_CAPTURE_H
#define CAMERA_CAPTURE_H
#include <QImage>
#include <QThread>
#include <QTimer>
#include <QWaitCondition>
#include <QMutex>
#include <time.h>
#include <sys/types.h>
#include "opencv2/opencv.hpp"
#include "opencv/highgui.h"
#include "opencv/cv.h"

class camera_capture : public QThread
{
        Q_OBJECT
public:
    camera_capture(int cameraID);
    void init(char *filename);
    void startRecord();
    void stopRecord();
    void stop();

signals:
    void imageReady(const QImage &image);
    void fpsReady(const double fps);
protected:
    void run() override;
private:
    const float frameRate = 30.0;
    cv::VideoCapture cap;
    cv::VideoWriter wrt;
    bool end, toRecord;
    cv::Mat img;
    time_t cTime, pTime;
    uint64_t cCnt, pCnt;

    //used for synchronizing thread
    QMutex mutex;
    QWaitCondition condition;
};

#endif // CAMERA_CAPTURE_H
