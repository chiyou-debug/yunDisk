#ifndef UPLOADLAYOUT_H
#define UPLOADLAYOUT_H
#include "common.h"
#include <QVBoxLayout>

// 上传进度布局类，单例模式
class UploadLayout
{
public:
    // 保证唯一一个实例
    static UploadLayout *getInstance();
    // 设置布局
    void setUploadLayout(QWidget *p);
    // 获取布局 - 添加控件
    QLayout *getUploadLayout();

private:
    UploadLayout();
    ~UploadLayout();

    //静态数据成员，类中声明，类外必须定义
    static UploadLayout *instance;
    QLayout *m_layout;
    QWidget *m_wg;
};

#endif // UPLOADLAYOUT_H
