#-------------------------------------------------
#
# Project created by QtCreator 2018-03-17T14:26:13
#
#-------------------------------------------------

QT       += core gui
CONFIG += console
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = record_v0_1
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
INCLUDEPATH += "C:\Program Files\opencv\build\include" \
"E:\Program Files (x86)\Intel\RSSDK\include" \
"E:\Program Files (x86)\Intel\RSSDK\sample\common\include" \
"C:\Users\Phree\Desktop\c++\record_v0_1\include"

LIBS += -lws2_32 \
-L"C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Lib\x64" \
-lkernel32 \
-luser32 \
-lgdi32 \
-lwinspool \
-lcomdlg32 \
-ladvapi32 \
-lshell32 \
-lole32 \
-loleaut32 \
-luuid \
-lodbc32 \
-lodbccp32 \
-L"C:\Users\Phree\Desktop\c++\record_v0_1\lib" \
-lmyo64

CONFIG(debug, debug|release): {
LIBS += -L"C:\Program Files\opencv\build\x64\vc14\lib" \
-lopencv_world331d \
-L"E:\Program Files (x86)\Intel\RSSDK\lib\x64" \
-llibpxc_d \
-L"E:\Program Files (x86)\Intel\RSSDK\sample\common\lib\x64\v140" \
-llibpxcutils_d
-llibpxcutilsmd_d
} else:CONFIG(release, debug|release): {
LIBS += -L"C:\Program Files\opencv\build\x64\vc14\lib" \
-lopencv_world331 \
-L"E:\Program Files (x86)\Intel\RSSDK\lib\x64" \
-llibpxc \
-L"E:\Program Files (x86)\Intel\RSSDK\sample\commonlib\x64\v140" \
-llibpxcutils
-llibpxcutilsmd
}

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    utils.cpp \
    tcp_reciever.cpp \
    myodatacollector.cpp

HEADERS += \
        mainwindow.h \
    tcp_controller.h \
    ../tcp_controller/tcp_controller_v0.1/tcp_controller_v0.1/tcp_controller.h \
    tcp_reciever.h

FORMS += \
        mainwindow.ui
