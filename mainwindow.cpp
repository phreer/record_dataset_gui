/*
 * Author: Phree Liu
 * Created Date: Mar.24.2018
 * Version: 0.1.0
 * used for managing camera, smart watch, myo and realsense
 */
#pragma comment(lib, "Ws2_32.lib") //add the dependency file for winsock2

#include "mainwindow.h"
#include "ui_mainwindow.h"

/*
 * used for acquiring a file name to save video stream
 */

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->hintLabel->setText("Click start to start record progress.");
    image = new QImage();

    //setup winsock2
    WSADATA wsa_data = { 0 };
    WORD wsa_version = MAKEWORD(2, 2);
    int n_ret = WSAStartup(wsa_version, &wsa_data);
    if (n_ret != NO_ERROR) {
        printf("main: startup wsa failed!\n");
    }

    int cameraID1 = 1;
    int cameraID2 = 1;
    if(useCamera){
        camera1Thread = new camera_capture(cameraID1);
//        camera2Thread = new camera_capture(cameraID2);
    }
    if(useMyo){
        myoThread = new myothread();
    }
    if(useRealsense){
        realsenseThread = new realsense_capture();
    }
    connect(ui->startRecordProgressButton, SIGNAL(clicked(bool)), \
            this, SLOT(startRecordProgress()));
    connect(ui->stopRecordProgressButton, SIGNAL(clicked(bool)), \
            this, SLOT(stopRecordProgress()));
//    if(useWear){
//        recieverThread.start();
//    }
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::initAll(int volID, int restStateID, int gestureID){
    char filename[512];
    if(useCamera) {
        sprintf(filename, "camera_%d_vol_%d_restState_%d_geture_%d.avi", \
            1, volID, restStateID, gestureID);
        camera1Thread->init(filename);
        sprintf(filename, "camera_%d_vol_%d_restState_%d_geture_%d.avi", \
            2, volID, restStateID, gestureID);
//        camera2Thread->init(filename);
    }
    if(useMyo) {
        sprintf(filename, "myo_vol_%d_restState_%d_geture_%d.csv", \
            volID, restStateID, gestureID);
        myoThread->init(filename);
    }
    if(useRealsense) {
        sprintf(filename, "realsense_vol_%d_restState_%d_geture_%d", \
            volID, restStateID, gestureID);
        realsenseThread->init(filename);
    }
}

void MainWindow::startAll(){
    if(useCamera) {
        startCamera();
    }
    if(useMyo) {
        startMyo();
    }
    if(useRealsense) {
        startRealsense();
    }
    if(useWear) {
        startWear();
    }
}

void MainWindow::stopAll(){
    if(useCamera) stopCamera();
    if(useRealsense) stopRealsense();
    if(useMyo) stopMyo();
    if(useWear) stopWearandRecv();
}

void MainWindow::startRecordProgress(){
    int volNum = 10;
    ui->startRecordProgressButton->setDisabled(true);
    ui->stopRecordProgressButton->setEnabled(true);
    for(int volID=0; volID<volNum; volID++){
        ui->volIDLabel->setText(QString::number(volID));
        recordVol(volID);
        ui->hintLabel->setText(QString("All done! Thank you!"));
    }
    ui->hintLabel->setText("All volunteers have been recorded!");
    ui->startRecordProgressButton->setEnabled(true);
    ui->stopRecordProgressButton->setDisabled(true);
}

void MainWindow::stopRecordProgress(){
    stopAll();
    ui->hintLabel->setText("Click start to start record progress.");
    ui->startRecordProgressButton->setEnabled(true);
    ui->stopRecordProgressButton->setDisabled(true);
}

void MainWindow::recordVol(int id){
    int restStateNum = 3;
    int gestureNum = 10;
    QTime time;
    time.start();
    for(int restStateID=0; restStateID<restStateNum; restStateID++){
        ui->restStateIDLabel->setText(QString::number(restStateID));
        ui->hintLabel->setText(QString("Rest State ")+QString::number(restStateID));
        time.restart();
        while(time.elapsed()<20000){
            QCoreApplication::processEvents();
        }
        for(int gestureID=0; gestureID<gestureNum; gestureID++){
            ui->gestureIDLabel->setText(QString::number(gestureID));
            ui->hintLabel->setText("Ready");
            time.restart();
            // wait for 5s to record and stay responsible for evnets
            while(time.elapsed()<5000){
                QCoreApplication::processEvents();
            }
            ui->hintLabel->setText("Recording...");
            recordUnit(id, restStateID, gestureID);
        }
    }
}

void MainWindow::recordUnit(int volID, int restStateID, int gestureID){
    QTime time;
    initAll(volID, restStateID, gestureID);
    startAll();
    time.restart();
    while(time.elapsed()<20000){
        QCoreApplication::processEvents();
    }
    stopAll();
}

void MainWindow::startMyo(){
    myoThread->startRecord();
}

void MainWindow::stopMyo(){
    myoThread->stopRecord();
}

void MainWindow::startCamera(){
    camera1Thread->startRecord();
//    camera2Thread->stopRecord();
    connect(camera1Thread, SIGNAL(imageReady(QImage)), this, SLOT(updateCamera1(QImage)));
    connect(camera1Thread, SIGNAL(fpsReady(double)), this, SLOT(updateFPSCamera1(double)));
//    connect(camera2Thread, SIGNAL(imageReady(QImage)), this, SLOT(updateUIlabel3(QImage)));
}

void MainWindow::stopCamera(){
    disconnect(camera1Thread, SIGNAL(imageReady(QImage)), this, SLOT(updateCamera1(QImage)));
    disconnect(camera1Thread, SIGNAL(fpsReady(double)), this, SLOT(updateFPSCamera1(double)));
//    disconnect(camera2Thread, SIGNAL(imageReady(QImage)), this, SLOT(updateUIlabel3(QImage)));
    camera1Thread->stopRecord();
//    camera2Thread->stopRecord();
}

/*
 * start a new thread to record realsense stream
 */
void MainWindow::startRealsense(){
    realsenseThread->startRecord();
    connect(realsenseThread, SIGNAL(imageReady(QImage)), this, SLOT(updateRealsense(QImage)));
}

void MainWindow::stopRealsense(){
    disconnect(realsenseThread, SIGNAL(imageReady(QImage)), this, SLOT(updateRealsense(QImage)));
    realsenseThread->stopRecord();
}


/*
 * Display captured image on UI
 */
void MainWindow::updateCamera1(const QImage &image){
    ui->camera1Previw->setPixmap(QPixmap::fromImage(image.scaled(ui->camera1Previw->size())));
    ui->camera1Previw->show();
}
void MainWindow::updateRealsense(const QImage &image){
    ui->realsensePreview->setPixmap(QPixmap::fromImage(image.scaled(ui->camera1Previw->size())));
    ui->realsensePreview->show();
}

void MainWindow::updateFPSCamera1(const double fps){
    ui->camera1FPSValue->setText(QString::number(fps));
}

void MainWindow::closeEvent(QCloseEvent *event){
    if(useCamera){
        camera1Thread->stop();
//        camera2Thread->stop();
    }
    if(useRealsense){
        realsenseThread->stop();
    }
    if(useMyo){
        myoThread->stop();
    }
//    if(useWear) disconnect2Wear();
}

void MainWindow::startWear(){
    if(!connected) connect2Wear();
    if(connected) send_command(serv_sock, start_c);
}

void MainWindow::stopWearandRecv(){
    //request smart watch to transfer file
    if(connected){
        send_command(serv_sock, stop_c);
        send_command(serv_sock, send_c);
    }
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
                connected = false;
                return connected;
            }
        }
        if (::connect(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
            printf("main: unable to connect to remote server. %ld\n", WSAGetLastError());
            connected = false;
        }else{
            connected = true;
            send_command(serv_sock, hello_c);
            if(!recieveServOn) {
                recieverThread.start();
                recieveServOn = true;
            }
            if(!recieverThread.isRunning()) recieverThread.start();
        }
        return connected;
    }
}

void MainWindow::disconnect2Wear(){
    if(serv_sock && connected) send_command(serv_sock, bye_c);
    if(recieverThread.isRunning()) recieverThread.stop();
    connected = false;
}

