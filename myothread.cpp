#include "myothread.h"

myothread::myothread(){
    end = false;
    hub = NULL;
    initMyo();
}

void myothread::stop(){
    mutex.lock();
    end = true;
    condition.wakeOne();
    mutex.unlock();
}

void myothread::init(const char *filename){
    if(ofile.is_open()) ofile.close();
    ofile.open(filename, std::ios::out | std::ios::app);
}

void myothread::startRecord(){
    mutex.lock();
    toRecord = true;
    mutex.unlock();

    if(!isRunning()){
        start();
    }else{
        mutex.lock();
        condition.wakeOne();
        mutex.unlock();
    }
}

void myothread::stopRecord(){
    toRecord = false;
}

void myothread::run(){
    for(;;){
        if(!toRecord){
            mutex.lock();
            condition.wait(&mutex);
            mutex.unlock();
        }
        if(!end){
            hub->run(1000/sample_rate);
            collector.print();
            collector.write(ofile);
        }else{
            ofile.close();
            delete hub;
            printf("myothread terminated.\n");
            return;
        }
    }
}

void myothread::initMyo(){
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
