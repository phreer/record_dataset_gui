#ifndef REALSENSE_CAPTURE_H
#define REALSENSE_CAPTURE_H

#include <QObject>
#include <QThread>
#include <QImage>
#include "pxcsensemanager.h"
#include <sys/types.h>
#include "opencv2/opencv.hpp"
#include "opencv/highgui.h"
#include "opencv/cv.h"
#include "time.h"

class realsense_capture : public QThread
{
    Q_OBJECT
public:
    realsense_capture(const char filename_color[], const char filename_depth[]);
    void stop();
signals:
    void imageReady(const QImage &image);
protected:
    void run() override;
private:
    bool end;
    cv::VideoWriter writerColorRe, writerDepthRe;
    cv::Mat frameColor;
    cv::Mat frameDepth;
    PXCSenseManager *pxcSenseManager;
    time_t cTime, pTime;
    uint64_t cCnt, pCnt;
    const cv::Size size = cv::Size(640, 480);
    const float frameRate = 30;
};

#endif // REALSENSE_CAPTURE_H
