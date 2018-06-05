#include "camera_capture.h"

camera_capture::camera_capture(int cameraID){
    if(!cap.isOpened())
        if(!cap.open(cameraID)){
            printf("camera_capture: unable to open camera!");
        }

    pCnt = cCnt = 0;
    time(&pTime);
    end = false;
    toRecord = false;
}

void camera_capture::init(char *filename){
    mutex.lock();
    if(wrt.isOpened()) wrt.release();
    wrt.open(filename, CV_FOURCC('D','I','V','X'), frameRate, \
             cv::Size(int(cap.get(CV_CAP_PROP_FRAME_WIDTH)), \
                      int(cap.get(CV_CAP_PROP_FRAME_HEIGHT))),\
             true);
    mutex.unlock();
    if(wrt.isOpened()) printf("Video writer opened.\n");
    else printf("Unable to open video writer.\n");
}

void camera_capture::startRecord(){
    toRecord = true;
    end = false;
    if(!isRunning()){
        start();
    }else{
        mutex.lock();
        condition.wakeOne();
        mutex.unlock();
    }
}

void camera_capture::stopRecord(){
    toRecord = false;
}


void camera_capture::stop(){
    mutex.lock();
    end = true;
    condition.wakeOne();
    mutex.unlock();
}

void camera_capture::run(){
    for(;;){
        if(!toRecord){
            mutex.lock();
            if(wrt.isOpened()) wrt.release();
            printf("camera thread is sleeping now.\n");
            condition.wait(&mutex);
            mutex.unlock();
        }

        if(!end){
            if(cap.isOpened()){
                cap >> img;
                if(!img.empty()){
                    mutex.lock();
                    wrt << img;
                    mutex.unlock();
                    mutex.lock();
                    cv::cvtColor(img, img, CV_BGR2RGB);
                    cv::flip(img, img, 1);
                    QImage image((const uchar*) (img.data), img.cols, img.rows, QImage::Format_RGB888);
                    emit imageReady(image);
                    mutex.unlock();
                    cCnt ++;
                    if((cCnt-pCnt)==200){
                        time(&cTime);
                        double fps = (cCnt-pCnt)/(cTime-pTime);
                        emit fpsReady(fps);
                        printf("Camera capture fps: %f\n", fps);
                        pCnt = cCnt;
                        pTime = cTime;
                    }
                }
            }



        }else{ //terminate this thread
            if(cap.isOpened())cap.release();
            if(wrt.isOpened())wrt.release();
            printf("Camera_thread terminated.\n");
            return;
        }
    }
}
