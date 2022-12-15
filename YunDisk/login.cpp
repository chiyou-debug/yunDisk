#include "login.h"
#include "ui_login.h"
#include <QPainter>
#include <QDebug>
#include <QMessageBox>
#include <QRegExp>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QGraphicsDropShadowEffect>
#include "common/logininfoinstance.h"

Login::Login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);

    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect(this);
    effect->setColor(Qt::black);
    effect->setOffset(-5, 5);
    effect->setBlurRadius(8);
    this->setGraphicsEffect(effect);

    // 初始化
    ui->titleBar->setMyParent(this);
    // 此处无需指定父窗口
    m_mainWin = new MainWindow;
    // 窗口图标
    this->setWindowIcon(QIcon(":/images/logo.ico"));
    m_mainWin->setWindowIcon(QIcon(":/images/logo.ico"));
    // 去掉边框
    this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);
    // 密码
    ui->passwd_login->setEchoMode(QLineEdit::Password);
    ui->passwd_reg->setEchoMode(QLineEdit::Password);
    ui->confirmPwd->setEchoMode(QLineEdit::Password);
    // 当前显示的窗口
    ui->stackedWidget->setCurrentIndex(0);
    ui->userName_login->setFocus();
    // 读取配置文件信息，并初始化
    readCfg();
    // 加载图片信息 - 显示文件列表的时候用，在此初始化
    m_cm.getFileTypeList();
    // 设置控件数据校验
    controlsVerify();

#ifdef DEBUG
    // 测试数据
    ui->userName_reg->setText("kevin_666");
    ui->nickName->setText("kevin@666");
    ui->passwd_reg->setText("123456");
    ui->confirmPwd->setText("123456");
    ui->phone->setText("11111111111");
    ui->email->setText("abc@qq.com");
#endif
    ui->address->setText("192.168.87.253");
    ui->port->setText("80");
    // 注册
    connect(ui->regAccount, &QToolButton::clicked, [=]()
    {
        ui->stackedWidget->setCurrentWidget(ui->regPage);
        ui->userName_reg->setFocus();
    });
    // 设置按钮
    connect(ui->titleBar, &MyTitleBar::showSetWindow, [=]()
    {
        ui->stackedWidget->setCurrentWidget(ui->serverPage);
        ui->address->setFocus();
    });
    // 关闭按钮
    connect(ui->titleBar, &MyTitleBar::closeWindow, this, &Login::windowClose);
    connect(ui->titleBar, &MyTitleBar::showMinWidnow, this, &Login::showMinimized);
    // 切换用户 - 重新登录
    connect(m_mainWin, &MainWindow::changeUser, [=]()
    {
        m_mainWin->hide();
        this->show();
    });
    // 窗口按钮事件处理
    connect(ui->loginButton, &QPushButton::clicked, this, &Login::userLogin);
    connect(ui->registreBtn, &QPushButton::clicked, this, &Login::registreUser);
    connect(ui->setOkBtn, &QPushButton::clicked, this, &Login::serverSet);
}

Login::~Login()
{
    delete ui;
}

// 用户登录操作
void Login::userLogin()
{
    // 获取用户登录信息
    QString user = ui->userName_login->text();
    QString pwd = ui->passwd_login->text();
    QString address = ui->address->text();
    QString port = ui->port->text();

    // 登录信息写入配置文件cfg.json
    // 登陆信息加密
    m_cm.writeLoginInfo(user, pwd, ui->rememberPwd->isChecked());
    // 设置登陆信息json包, 密码经过md5加密， getStrMd5()
    QStringList keys = QStringList() << "user" << "pwd";
    QStringList values  =QStringList() << user << m_cm.getMD5(pwd);
    QByteArray json = m_cm.getJsonString(keys, values);
    // 设置登录的url
    QNetworkRequest request;
    QString url = QString("http://%1:%2/login").arg(address).arg(port);
    request.setUrl(QUrl(url));
    // 请求头信息
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
    request.setHeader(QNetworkRequest::ContentLengthHeader, QVariant(json.size()));
    // 向服务器发送post请求
    QNetworkReply* reply = Common::getNetWorkManager()->post(request, json);
    cout << "post url:" << url << "post data: " << json;

    // 接收服务器发回的http响应消息
    connect(reply, &QNetworkReply::finished, [=]()
    {
        // 出错了
        if (reply->error() != QNetworkReply::NoError)
        {
            cout << reply->errorString();
            //释放资源
            reply->deleteLater();
            return;
        }

        // 将server回写的数据读出
        QByteArray json = reply->readAll();
        /*
            登陆 - 服务器回写的json数据包格式：
                成功：{"code":"000"}
                失败：{"code":"001"}
        */
        cout << "server return value: " << json;
        QJsonDocument doc = QJsonDocument::fromJson(json);
        QJsonObject obj = doc.object();
        if( obj.value("code").toString() == "000" )
        {
            cout << "登陆成功";

            // 设置登陆信息，显示文件列表界面需要使用这些信息
            LoginInfoInstance *p = LoginInfoInstance::getInstance(); //获取单例
            p->setLoginInfo(user, address, port, obj.value("token").toString());

            // 当前窗口隐藏
            this->hide();
            // 主界面窗口显示
            m_mainWin->showMainWindow();
        }
        else
        {
            QMessageBox::warning(this, "登录失败", "用户名或密码不正确！！！");
        }

        reply->deleteLater(); //释放资源
    });
}

// 用户注册操作
void Login::registreUser()
{
    // 从控件中取出用户输入的数据
    QString userName = ui->userName_reg->text();
    QString nickName = ui->nickName->text();
    QString firstPwd = ui->passwd_reg->text();
    QString surePwd = ui->confirmPwd->text();
    QString phone = ui->phone->text();
    QString email = ui->email->text();

    if(firstPwd != surePwd)
    {
        QMessageBox::warning(this, "Warning", "两次输入的密码不一致!");
        return;
    }

    // 将注册信息打包为json格式
    QStringList keys = QStringList() << "userName" << "nickName" << "firstPwd" << "phone" << "email";
    QStringList values = QStringList() << userName << nickName << m_cm.getMD5(firstPwd) << phone << email;
    QByteArray json = m_cm.getJsonString(keys, values);
    qDebug() << "register json data" << json;
    // 设置连接服务器要发送的url
    QNetworkRequest request;
    QString url = QString("http://%1:%2/reg").arg(ui->address->text()).arg(ui->port->text());
    request.setUrl(QUrl(url));
    // 设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
    request.setHeader(QNetworkRequest::ContentLengthHeader, QVariant(json.size()));
    // 发送数据
    QNetworkReply* reply = Common::getNetWorkManager()->post(request, json);

    // 判断请求是否被成功处理
    connect(reply, &QNetworkReply::readyRead, [=]()
    {
        // 读sever回写的数据
        QByteArray jsonData = reply->readAll();
        /*
            注册 - server端返回的json格式数据：
            成功:         {"code":"002"}
            该用户已存在：  {"code":"003"}
            失败:         {"code":"004"}
        */
        // 判断状态码
        if("002" == m_cm.getCode(jsonData))
        {
            //注册成功
            QMessageBox::information(this, "注册成功", "注册成功，请登录");

            //清空行编辑内容
            ui->userName_reg->clear();
            ui->nickName->clear();
            ui->passwd_reg->clear();
            ui->confirmPwd->clear();
            ui->phone->clear();
            ui->email->clear();

            //设置登陆窗口的登陆信息
            ui->userName_login->setText(userName);
            ui->passwd_login->setText(firstPwd);
            ui->rememberPwd->setChecked(true);

            //切换到登录界面
            ui->stackedWidget->setCurrentWidget(ui->loginPage);
        }
        else if("003" == m_cm.getCode(jsonData))
        {
            QMessageBox::warning(this, "注册失败", QString("[%1]该用户已经存在!!!").arg(userName));
        }
        else if("004" == m_cm.getCode(jsonData))
        {
            QMessageBox::warning(this, "注册失败", "注册失败！！！");
        }
        // 释放资源
        reply->deleteLater();
    });
}

// 用户设置操作
void Login::serverSet()
{
    QString ip = ui->address->text();
    QString port = ui->port->text();

    // 数据判断
    // 服务器IP
    // \\d 和 \\. 中第一个\是转义字符, 这里使用的是标准正则

    // 跳转到登陆界面
    ui->stackedWidget->setCurrentWidget(ui->loginPage);
    // 将配置信息写入配置文件中
    m_cm.writeWebInfo(ip, port);
}

void Login::windowClose()
{

    // 如果是登录窗口
    if(ui->stackedWidget->currentWidget() == ui->loginPage)
    {
        close();
    }
    // 如果是注册窗口
    else if(ui->stackedWidget->currentWidget() == ui->regPage)
    {
        // 清空数据
        ui->email->clear();
        ui->userName_reg->clear();
        ui->nickName->clear();
        ui->passwd_reg->clear();
        ui->confirmPwd->clear();
        ui->phone->clear();
    }
    // 如果是设置窗口
    else
    {
        // 清空数据
        ui->address->clear();
        ui->port->clear();
    }
    // 窗口切换
    ui->stackedWidget->setCurrentWidget(ui->loginPage);
    ui->userName_login->setFocus();
}

// 读取配置信息，设置默认登录状态，默认设置信息
void Login::readCfg()
{
#if 0
    QString user = m_cm.getCfgValue("login", "user");
    QString pwd = m_cm.getCfgValue("login", "pwd");
    QString remeber = m_cm.getCfgValue("login", "remember");
    int ret = 0;

    if(remeber == "yes")//记住密码
    {
        //密码解密
        unsigned char encPwd[512] = {0};
        int encPwdLen = 0;
        //toLocal8Bit(), 转换为本地字符集，默认windows则为gbk编码，linux为utf-8编码
        QByteArray tmp = QByteArray::fromBase64( pwd.toLocal8Bit());
        ret = DesDec( (unsigned char *)tmp.data(), tmp.size(), encPwd, &encPwdLen);
        if(ret != 0)
        {
            cout << "DesDec";
            return;
        }

    #ifdef _WIN32 //如果是windows平台
        //fromLocal8Bit(), 本地字符集转换为utf-8
        ui->passwd_login->setText( QString::fromLocal8Bit( (const char *)encPwd, encPwdLen ) );
    #else //其它平台
        ui->passwd_login->setText( (const char *)encPwd );
    #endif

        ui->rememberPwd->setChecked(true);

    }
    else //没有记住密码
    {
        ui->passwd_login->setText("");
        ui->rememberPwd->setChecked(false);
    }

    //用户解密
    unsigned char encUsr[512] = {0};
    int encUsrLen = 0;
    //toLocal8Bit(), 转换为本地字符集，如果windows则为gbk编码，如果linux则为utf-8编码
    QByteArray tmp = QByteArray::fromBase64( user.toLocal8Bit());
    ret = DesDec( (unsigned char *)tmp.data(), tmp.size(), encUsr, &encUsrLen);
    if(ret != 0)
    {
        cout << "DesDec";
        return;
    }

    #ifdef _WIN32 //如果是windows平台
        //fromLocal8Bit(), 本地字符集转换为utf-8
        ui->userName_login->setText( QString::fromLocal8Bit( (const char *)encUsr, encUsrLen ) );
    #else //其它平台
        ui->userName_login->setText( (const char *)encUsr );
    #endif

    QString ip = m_cm.getCfgValue("web_server", "ip");
    QString port = m_cm.getCfgValue("web_server", "port");
    ui->address->setText(ip);
    ui->port->setText(port);
#endif
}

void Login::controlsVerify()
{
    // 数据的格式提示
    ui->userName_login->setToolTip("合法字符:[a-z|A-Z|#|@|0-9|-|_|*],字符个数: 3~16");
    ui->userName_reg->setToolTip("合法字符:[a-z|A-Z|#|@|0-9|-|_|*],字符个数: 3~16");
    ui->nickName->setToolTip("合法字符:[a-z|A-Z|#|@|0-9|-|_|*],字符个数: 3~16");
    ui->passwd_login->setToolTip("合法字符:[a-z|A-Z|#|@|0-9|-|_|*],字符个数: 6~18");
    ui->passwd_reg->setToolTip("合法字符:[a-z|A-Z|#|@|0-9|-|_|*],字符个数: 6~18");
    ui->confirmPwd->setToolTip("合法字符:[a-z|A-Z|#|@|0-9|-|_|*],字符个数: 6~18");

    // 登录
    // 用户名
    QRegExp regexp;
    regexp.setPattern(USER_REG);
    QRegExpValidator *userReg  = new QRegExpValidator(regexp, this);
    ui->userName_login->setValidator(userReg);
    // 密码
    regexp.setPattern(PASSWD_REG);
    QRegExpValidator *pwdReg  = new QRegExpValidator(regexp, this);
    ui->passwd_login->setValidator(pwdReg);

    // 注册
    // 用户名/昵称/密码/确认密码
    ui->userName_reg->setValidator(userReg);
    ui->nickName->setValidator(userReg);
    ui->passwd_reg->setValidator(pwdReg);
    ui->confirmPwd->setValidator(pwdReg);
    // 手机
    regexp.setPattern(PHONE_REG);
    QRegExpValidator *phoneReg  = new QRegExpValidator(regexp, this);
    ui->phone->setValidator(phoneReg);
    // 邮箱
    regexp.setPattern(EMAIL_REG);
    QRegExpValidator *emailReg  = new QRegExpValidator(regexp, this);
    ui->email->setValidator(emailReg);

    // 服务器设置
    regexp.setPattern(IP_REG);
    QRegExpValidator *ipReg  = new QRegExpValidator(regexp, this);
    ui->address->setValidator(ipReg);
    ui->address->setInputMask("000.000.000.000;0");

    // 端口号
    regexp.setPattern(PORT_REG);
    QRegExpValidator *portReg  = new QRegExpValidator(regexp, this);
    ui->port->setValidator(portReg);
}

void Login::on_loginButton_clicked()
{

}
