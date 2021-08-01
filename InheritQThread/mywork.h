#ifndef MYWORK_H
#define MYWORK_H

#include <QThread>
#include <QListWidget>

class MyWork : public QThread
{
public:
    MyWork(QListWidget *w, int buttonID, QObject *parent = nullptr);
    // 向列表添加 "<线程号> 按钮i: <1-5>"
    void run() override;

private:
    QListWidget *w;
    int buttonID;
};

#endif // MYWORK_H
