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
    image = new QImage();

    //setup winsock2
    WSADATA wsa_data = { 0 };
    WORD wsa_version = MAKEWORD(2, 2);
    int n_ret = WSAStartup(wsa_version, &wsa_data);
    if (n_ret != NO_ERROR) {
        printf("main: startup wsa failed!\n");
    }

    if(useCamera){
        connect(ui->startButton, SIGNAL(clicked()), this, SLOT(startCamera()));
        connect(ui->stopButton, SIGNAL(clicked()), this, SLOT(stopCamera()));
    }
    if(useWear){
        reciever_thread.start();
        connect(ui->startButton, SIGNAL(clicked()), this, SLOT(startWear()));
        connect(ui->stopButton, SIGNAL(clicked()), this, SLOT(stopWearandRecv()));
        connect(ui->connectButton, SIGNAL(clicked()), this, SLOT(connect2Wear()));
        connect(ui->disconnectButton, SIGNAL(clicked()), this, SLOT(disconnect2Wear()));
    }
    if(useMyo){
        connect(ui->startButton, SIGNAL(clicked(bool)), this, SLOT(startMyo()));
        connect(ui->stopButton, SIGNAL(clicked(bool)), this, SLOT(stopMyo()));
    }
    if(useRealsense){
        connect(ui->startButton, SIGNAL(clicked()), this, SLOT(startRealsense()));
        connect(ui->stopButton, SIGNAL(clicked(bool)), this, SLOT(stopRealsense()));
    }
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::startMyo(){
    myo_thread = new myothread("test.csv");
    myo_thread->start();
}

void MainWindow::stopMyo(){
    if(myo_thread->isRunning()) myo_thread->stop();
}

void MainWindow::startCamera(){
    char filename[] = "test.avi";
    camera_thread = new camera_capture(filename);
    camera_thread->start();
    connect(camera_thread, SIGNAL(imageReady(QImage)), this, SLOT(updateUIlabel1(QImage)));
}

void MainWindow::stopCamera(){
    disconnect(camera_thread, SIGNAL(imageReady(QImage)), this, SLOT(updateUIlabel1(QImage)));
    if(camera_thread->isRunning()) camera_thread->stop();
}

/*
 * start a new thread to record realsense stream
 */
void MainWindow::startRealsense(){
    char filename_color[] = "getFilename(REALSENSE_COLOR).avi";
    char filename_depth[] = "getFilename(REALSENSE_DEPTH).avi";

    realsense_thread = new realsense_capture(filename_color, filename_depth);
    realsense_thread->start();
    connect(realsense_thread, SIGNAL(imageReady(QImage)), this, SLOT(updateUIlabel2(QImage)));
}

void MainWindow::stopRealsense(){
    disconnect(realsense_thread, SIGNAL(imageReady(QImage)), this, SLOT(updateUIlabel2(QImage)));
    if(realsense_thread && realsense_thread->isRunning()) realsense_thread->stop();
}

/*
 * Display captured image on UI
 */
void MainWindow::updateUIlabel1(const QImage &image){
    ui->label->setPixmap(QPixmap::fromImage(image));
    ui->label->show();
}
void MainWindow::updateUIlabel2(const QImage &image){
    ui->label_2->setPixmap(QPixmap::fromImage(image));
    ui->label_2->show();
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



void MainWindow::disconnect2Wear(){
    if(serv_sock) send_command(serv_sock, bye_c);
    if(reciever_thread.isRunning()) reciever_thread.terminate();
}

