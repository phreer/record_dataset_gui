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
    if(useCamera || useRealsense){
        connect(timer, SIGNAL(timeout()), this, SLOT(readFrame()));
        connect(ui->startButton, SIGNAL(clicked()), this, SLOT(startTimer()));
        connect(ui->stopButton, SIGNAL(clicked()), this, SLOT(stopTimer()));
    }


    if(useCamera){
        connect(ui->startButton, SIGNAL(clicked()), this, SLOT(startCamera()));
        connect(ui->stopButton, SIGNAL(clicked()), this, SLOT(stopCamera()));
    }
    if(useWear){
        connect(ui->startButton, SIGNAL(clicked()), this, SLOT(startWear()));
        connect(ui->stopButton, SIGNAL(clicked()), this, SLOT(stopWearandRecv()));
        connect(ui->connectButton, SIGNAL(clicked()), this, SLOT(connect2Wear()));
        connect(ui->disconnectButton, SIGNAL(clicked()), this, SLOT(disconnect2Wear()));
    }
    if(useMyo){
        connect(ui->startButton, SIGNAL(clicked(bool)), this, SLOT(startMyo()));

    }
    if(useRealsense){
        connect(ui->startButton, SIGNAL(clicked()), this, SLOT(startRealsense()));
        connect(ui->stopButton, SIGNAL(clicked(bool)), this, SLOT(stopRealsense()));
    }
}

MainWindow::~MainWindow(){
    delete ui;
}

/*
 * used for recording frame after a specific time interval
 */
void MainWindow::startTimer(){
    timer->start(10);
}

void MainWindow::startMyo(){
try {
    // First, we create a Hub with our application identifier. Be sure not to use the com.example namespace when
    // publishing your application. The Hub provides access to one or more Myos.
    hub = new myo::Hub("com.example.emg-data-sample");
    std::cout << "Attempting to find a Myo..." << std::endl;
    // Next, we attempt to find a Myo to use. If a Myo is already paired in Myo Connect, this will return that Myo
    // immediately.
    // waitForMyo() takes a timeout value in milliseconds. In this case we will try to find a Myo for 10 seconds, and
    // if that fails, the function will return a null pointer.
    a_myo = hub->waitForMyo(10000);
    // If waitForMyo() returned a null pointer, we failed to find a Myo, so exit with an error message.
    if (!a_myo) {
        throw std::runtime_error("Unable to find a Myo!");
    }
    // We've found a Myo.
    std::cout << "Connected to a Myo armband!" << std::endl << std::endl;
    // Next we enable EMG streaming on the found Myo.
    a_myo->setStreamEmg(myo::Myo::streamEmgEnabled);
    // Hub::addListener() takes the address of any object whose class inherits from DeviceListener, and will cause
    // Hub::run() to send events to all registered device listeners.
    hub->addListener(&collector);
}catch(const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    std::cerr << "Press enter to continue.";
    std::cin.ignore();
}
}

void MainWindow::startCamera(){
    char FILE_NAME[] = "test.avi";
    if(!capture.isOpened()) capture.open(3);
    if(capture.isOpened()){
        double fps = capture.get(CV_CAP_PROP_FPS);
        printf("%f", fps);
        writer.open(FILE_NAME, CV_FOURCC('D','I','V','X'), frameRate, \
                    cv::Size(int(capture.get(CV_CAP_PROP_FRAME_WIDTH)), int(capture.get(CV_CAP_PROP_FRAME_HEIGHT))),\
                    true);
    }else{
        printf("Unable to open capture!");
    }
}

/*
 * record stream and preview on the screen
 * synchronized with Timer
 * in fact I am not sure whether this would work properly ?
 * seems my webcam can only support 30 fps
 */
void MainWindow::readFrame(){
    printf("readFrame called\n");
    if(useCamera){
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
    }


    if(useRealsense){
        //QuerySample function will NULL untill all frames are available
        //unless you set its param ifall false
        pxcSenseManager->AcquireFrame();
        PXCCapture::Sample *sample = pxcSenseManager->QuerySample();
        if(sample){
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

    }
//    if(a_myo){
//        // In each iteration of our main loop, we run the Myo event loop for a set number of milliseconds.
//        // In this case, we wish to update our display 100 times a second, so we run for 1000/10 milliseconds.
//        hub->run(1000 / 10);
//        // After processing events, we call the print() member function we defined above to print out the values we've
//        // obtained from any events that have occurred.
//        collector.print();
//        //collector.write();
//        collector.write();
//    }

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
    if(connected) send_command(serv_sock, start_c);
}



void MainWindow::stopWearandRecv(){
    //request smart watch to transfer file
    send_command(serv_sock, stop_c);
    send_command(serv_sock, send_c);
}


/*
 * setup a socket and try to connect to the server(smart watch)
 * connected will be set true if connect successfully
 * return:
 *      bool: true if connect successfully else false
 */
bool MainWindow::connect2Wear(){
    if(serv_sock==NULL || !send_command(serv_sock, hello_c)){
        memset(&send_buffer, 0, BUFFER_SIZE);
        memset(&recv_buffer, 0, BUFFER_SIZE);
        memset(&serv_addr, 0, sizeof(serv_addr));
        if(serv_sock==NULL){
            set_addr_s(&serv_addr, SERV_IP, SERV_PORT);
            serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (serv_sock == INVALID_SOCKET) {
                printf("main: get socket failed. %ld\n", WSAGetLastError());
            }
        }
        if (::connect(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
            printf("main: unable to connect to remote server. %ld\n", WSAGetLastError());
            connected = false;
        }else{
            connected = true;
            send_command(serv_sock, hello_c);
            if(!recieveServOn) {
                reciever_thread.start();
                recieveServOn = true;
            }
            if(!reciever_thread.isRunning()) reciever_thread.start();
        }

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
    char filename_depth[] = "getFilename(REALSENSE_DEPTH).avi";

    writerColorRe.open(filename_color, CV_FOURCC('D','I','V','X'), frameRate, size, true);
    writerIrRe.open(filename_ir, CV_FOURCC('D','I','V','X'), frameRate, size, false);
    writerDepthRe.open(filename_depth, CV_FOURCC('D','I','V','X'), frameRate, size, false);


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
    if(pxcSenseManager->Init() == pxcStatus::PXC_STATUS_NO_ERROR){
        printf("startRealsense: Init successfully.\n");
    }else{
        printf("startRealsense: Init failed!\n");
    }
    printf("startRealsense: Realsense Enabled\n");
}

void MainWindow::disconnect2Wear(){
    if(serv_sock) send_command(serv_sock, bye_c);
    if(reciever_thread.isRunning()) reciever_thread.terminate();
}

