#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <mainwindow.h>
#include "common/common.h"

namespace Ui {
class Login;
}

class Login : public QDialog
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = 0);
    ~Login();
    
    // 得到服务器回复的登陆状态， 状态码返回值为 "000", 或 "001"，还有登陆section
    QStringList getLoginStatus(QByteArray json);


private slots:
    void registreUser();
    void userLogin();
    void serverSet();
    void windowClose();

    void on_loginButton_clicked();

private:
    // 读取配置信息，设置默认登录状态，默认设置信息
    void readCfg();
    void controlsVerify();

private:
    Ui::Login *ui;

    // 主窗口指针
    MainWindow* m_mainWin;
    Common m_cm;
};

#endif // LOGIN_H
