/*
 * Author: Phree Liu
 * Created Date: Mar.24.2018
 * Version: 0.1.0
 * used for managing camera, smart watch, myo and realsense
 */
#pragma comment(lib, "Ws2_32.lib") //add the dependency file for winsock2


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utils.cpp"
#include "tcp_reciever.cpp"


/*
 * utility function to transfer realsense format image to opencv format Mat
 * params:
 *  pxcImage: pointer to PXCImage instance
 *  format: can be NULL or predefined PIXEL_FORMAT_... format
 * return:
 *  a desired Mat
 */
cv::Mat PXCImage2CVMat(PXCImage *pxcImage, PXCImage::PixelFormat format){
    PXCImage::ImageData data;
    pxcImage->AcquireAccess(PXCImage::ACCESS_READ, format, &data);
    int width = pxcImage->QueryInfo().width;
    int height = pxcImage->QueryInfo().height;

    if(!format)
        format = pxcImage->QueryInfo().format;
    int type;
    switch(format){
        case PXCImage::PIXEL_FORMAT_Y8:
            type = CV_8UC1;
            break;
        case PXCImage::PIXEL_FORMAT_RGB24:
            type = CV_8UC3;
            break;
        case PXCImage::PIXEL_FORMAT_DEPTH_F32:
            type = CV_32FC1;
            break;
    }
    cv::Mat ocvImage = cv::Mat(cv::Size(width, height), type, data.planes[0]);
    pxcImage->ReleaseAccess(&data);
    return ocvImage;
}

/*
 * used for acquiring a file name to save video stream
 */
char* getFilename(){
    char filename[] = "test.avi";
    return filename;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timer = new QTimer(this);
    image = new QImage();

    //setup winsock2
    WSADATA wsa_data = { 0 };
    WORD wsa_version = MAKEWORD(2, 2);
    int n_ret = WSAStartup(wsa_version, &wsa_data);
    if (n_ret != NO_ERROR) {
        printf("main: startup wsa failed!");
    }

    reciever_thread.start();
    //initialize Mat to store image from realsense
    frameIR = cv::Mat::zeros(size, CV_8UC1);
    frameColor = cv::Mat::zeros(size, CV_8UC3);
    frameDepth = cv::Mat::zeros(size, CV_8UC1);

    //connect signals to slots, which means when a signal sent(button clicked here), the corresponding slot will be called
    connect(timer, SIGNAL(timeout()), this, SLOT(readFrame()));
    connect(ui->startButton, SIGNAL(clicked()), this, SLOT(startCamera()));
    connect(ui->startButton, SIGNAL(clicked()), this, SLOT(startWear()));
    connect(ui->startButton, SIGNAL(clicked()), this, SLOT(startRealsense()));

    connect(ui->stopButton, SIGNAL(clicked()), this, SLOT(stopCamera()));
    connect(ui->stopButton, SIGNAL(clicked()), this, SLOT(stopWearandRecv()));
    connect(ui->stopButton, SIGNAL(clicked()), this, SLOT(stopTimer()));

    connect(ui->connectButton, SIGNAL(clicked()), this, SLOT(connect2Wear()));
    connect(ui->disconnectButton, SIGNAL(clicked()), this, SLOT(disconnect2Wear()));

}

MainWindow::~MainWindow(){
    delete ui;
}

/*
 * used for recording frame after a specific time interval
 */
void MainWindow::startTimer(){
    timer->start(1000/frameRate);
}

void MainWindow::startCamera(){
    char *FILE_NAME = getFilename();
    if(!capture.isOpened()) capture.open(0);
    writer.open(FILE_NAME, CV_FOURCC('D','I','V','X'), frameRate, \
                cv::Size(int(capture.get(CV_CAP_PROP_FRAME_WIDTH)), int(capture.get(CV_CAP_PROP_FRAME_HEIGHT))),\
                true);

}

/*
 * record stream and preview on the screen
 * synchronized with Timer
 * in fact I am not sure whether this would work properly ?
 */
void MainWindow::readFrame()
{

    if(capture.isOpened()){
        capture >> img;
    }
    if(!img.empty()){
        cv::cvtColor(img, img, CV_BGR2RGB);
        cv::flip(img, img, 1);
        writer << img;
        image = new QImage((const uchar*) (img.data), img.cols, img.rows, QImage::Format_RGB888);
        ui->label->setPixmap(QPixmap::fromImage(*image));
        ui->label->show();
    }

    //this function will be blocked untill all frames are available
    //unless you set its param wait_all false
    pxcSenseManager->AcquireFrame(true);
    PXCCapture::Sample *sample = pxcSenseManager->QuerySample();

    frameIR = PXCImage2CVMat(sample->ir, PXCImage::PIXEL_FORMAT_Y8);
    frameColor = PXCImage2CVMat(sample->color, PXCImage::PIXEL_FORMAT_RGB24);
    PXCImage2CVMat(sample->depth, PXCImage::PIXEL_FORMAT_DEPTH_F32).convertTo(frameDepth, CV_8UC1);

    writerIrRe << frameIR;
    writerColorRe << frameColor;
    writerDepthRe << frameDepth;

    //show depth image in label2
    label2Image = new QImage((const uchar*)(frameDepth.data), \
                             frameDepth.cols, \
                             frameDepth.rows, \
                             QImage::Format_Grayscale8);
    ui->label_2->setPixmap(QPixmap::fromImage(*label2Image));
    ui->label_2->show();

    //clean up consumed frame
    pxcSenseManager->ReleaseFrame();
}

void MainWindow::stopTimer(){
    timer->stop();
}
void MainWindow::stopCamera(){
    if (capture.isOpened()){
        capture.release();
    }
    if(writer.isOpened()){
        writer.release();
    }
}
void MainWindow::stopRealsense(){
    if(pxcSenseManager->IsConnected()) pxcSenseManager->Release();
    if(writerIrRe.isOpened()) writerIrRe.release();
    if(writerColorRe.isOpened()) writerColorRe.release();
    if(writerDepthRe.isOpened()) writerDepthRe.release();
}

//test util, deprecated
void MainWindow::takingPictures(){
    image = new QImage((const uchar*) (img.data), img.cols, img.rows, QImage::Format_RGB888);
    ui->label_2->setPixmap(QPixmap::fromImage(*image));
}

void MainWindow::startWear(){
    if(!connected) connect2Wear();
    send_command(serv_sock, start_c);
}



void MainWindow::stopWearandRecv(){
    //request smart watch to transfer file
    send_command(serv_sock, send_c);
}

bool MainWindow::connect2Wear(){
    if(serv_sock==NULL || !send_command(serv_sock, hello_c)){
        memset(&send_buffer, 0, BUFFER_SIZE);
        memset(&recv_buffer, 0, BUFFER_SIZE);
        memset(&serv_addr, 0, sizeof(serv_addr));
        if(serv_sock==NULL){
            set_addr_s(&serv_addr, SERV_IP, SERV_PORT);
            serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (serv_sock == INVALID_SOCKET) {
                printf("main: get socket failed. %ld", WSAGetLastError());
            }
        }
        if (::connect(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
            printf("main: unable to connect to remote server. %ld", WSAGetLastError());
            connected = false;
        }
        connected = true;
        send_command(serv_sock, hello_c);
        if(!recieveServOn) {
            reciever_thread.start();
            recieveServOn = true;
        }
        if(!reciever_thread.isRunning()) reciever_thread.start();
        return connected;
    }
}

/*
 * initialize Realsense
 * refer to Realsense SDK
 */
void MainWindow::startRealsense(){
    char filename_color[] = "getFilename(REALSENSE_COLOR).avi";
    char filename_ir[] = "getFilename(REALSENSE_IR).avi";
    char filename_depth[] = "getFilename(REALSENSE_COLOR).avi";

    writerColorRe.open(filename_color, CV_FOURCC('D','I','V','X'), frameRate, size, true);
    writerIrRe.open(filename_ir, CV_FOURCC('D','I','V','X'), frameRate, size, true);
    writerDepthRe.open(filename_depth, CV_FOURCC('D','I','V','X'), frameRate, size, true);


    pxcSenseManager = PXCSenseManager::CreateInstance();
    pxcSenseManager->EnableStream(PXCCapture::STREAM_TYPE_IR, \
                                  size.width, \
                                  size.height, \
                                  frameRate);
    pxcSenseManager->EnableStream(PXCCapture::STREAM_TYPE_COLOR, \
                                  size.width, \
                                  size.height, \
                                  frameRate);
    pxcSenseManager->EnableStream(PXCCapture::STREAM_TYPE_DEPTH, \
                                  size.width, \
                                  size.height, \
                                  frameRate);
    pxcSenseManager->Init();
}

void MainWindow::disconnect2Wear(){
    if(serv_sock) send_command(serv_sock, bye_c);
    if(reciever_thread.isRunning()) reciever_thread.terminate();
}

