#include "recordvideo.h"
#include "mainwindow.h"
RecordVideo::RecordVideo()
{

}

void RecordVideo::run()
{
    CvCapture * capture;
    capture = cvCreateCameraCapture(0);
    assert (capture!=NULL);

    IplImage *bgr_frame = cvQueryFrame(capture);
    double fps = cvGetCaptureProperty(capture, CV_CAP_PROP_FPS);
    CvSize size = cvSize((int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH), \
                         (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH));
    CvVideoWriter *writer = cvCreateVideoWriter(FILE_NAME, CV_FOURCC('M','J','P','G'),fps, size);
    IplImage * logpolar_frame = cvCreateImage(size, IPL_DEPTH_8U, 3);
    while(bgr_frame=cvQueryFrame(capture!=NULL)){
        cvLogPolar(bgr_frame, logpolar_frame, cvPoin);
    }
}
