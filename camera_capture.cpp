#include "camera_capture.h"

camera_capture::camera_capture(const char filename[]){
    if(!cap.isOpened()) cap.open(4);
    if(!wrt.isOpened()) wrt.open(filename, CV_FOURCC('D','I','V','X'), frameRate, \
                                 cv::Size(int(cap.get(CV_CAP_PROP_FRAME_WIDTH)), \
                                          int(cap.get(CV_CAP_PROP_FRAME_HEIGHT))),\
                                 true);
    pCnt = cCnt = 0;
    time(&pTime);
    end = false;
}

void camera_capture::stop(){
    end = true;
}

void camera_capture::run(){
    for(;;){
        if(!end){
            if(cap.isOpened()){
                cap >> img;
            }
            if(!img.empty()){
                wrt << img;
                cv::cvtColor(img, img, CV_BGR2RGB);
                cv::flip(img, img, 1);
                QImage image((const uchar*) (img.data), img.cols, img.rows, QImage::Format_RGB888);
                emit imageReady(image);
                cCnt ++;
                if((cCnt-pCnt)==200){
                    time(&cTime);
                    double fps = (cCnt-pCnt)/(cTime-pTime);
                    printf("Camera capture fps: %f\n", fps);
                    pCnt = cCnt;
                    pTime = cTime;
                }
            }
        }else{
            if(cap.isOpened())cap.release();
            if(wrt.isOpened())wrt.release();
            printf("Camera_thread terminated.\n");
            return;
        }
    }
}
