#-------------------------------------------------
#
# Project created by QtCreator 2017-07-07T17:42:16
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = YunDisk
TEMPLATE = app
RC_ICONS = ./images/logo.ico


SOURCES += main.cpp\
        mainwindow.cpp \
    login.cpp \
    common/common.cpp \
    common/logininfoinstance.cpp \
    buttongroup.cpp \
    myfilewg.cpp \
    sharelist.cpp \
    common/uploadtask.cpp \
    selfwidget/dataprogress.cpp \
    common/downloadlayout.cpp \
    common/uploadlayout.cpp \
    selfwidget/filepropertyinfo.cpp \
    common/downloadtask.cpp \
    transfer.cpp \
    rankinglist.cpp \
    selfwidget/mymenu.cpp \
    selfwidget/mytitlebar.cpp

HEADERS  += mainwindow.h \
    login.h \
    global.h \
    common/common.h \
    common/logininfoinstance.h \
    buttongroup.h \
    myfilewg.h \
    sharelist.h \
    common/uploadtask.h \
    selfwidget/dataprogress.h \
    common/downloadlayout.h \
    common/uploadlayout.h \
    selfwidget/filepropertyinfo.h \
    common/downloadtask.h \
    transfer.h \
    rankinglist.h \
    selfwidget/mymenu.h \
    selfwidget/mytitlebar.h

FORMS    += mainwindow.ui \
    login.ui \
    buttongroup.ui \
    myfilewg.ui \
    sharelist.ui \
    selfwidget/dataprogress.ui \
    selfwidget/filepropertyinfo.ui \
    transfer.ui \
    rankinglist.ui \
    selfwidget/mytitlebar.ui

RESOURCES += \
    resource.qrc
