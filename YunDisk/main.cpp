#include "mainwindow.h"
#include <QApplication>
#include "login.h"
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Login w;

    QFile file("D:\\CodeProject\\YunDisk\\style.qss");
    file.open(QFile::ReadOnly);
    a.setStyleSheet(file.readAll());
    file.close();

    w.show();

    return a.exec();
}

