#ifndef MYWORK_H
#define MYWORK_H

#include <QRunnable>
#include <QListWidget>
#include <QThread>

class MyWork : public QRunnable
{
public:
    MyWork(QListWidget *w, int buttonID);
    // 向列表添加 "<线程号> 按钮i: <1-5>"
    void run() override;

private:
    QListWidget *w;
    int buttonID;
};

#endif // MYWORK_H
