#include "mywork.h"

MyWork::MyWork(QListWidget *w, int buttonID, QObject *parent)
    : QThread(parent)
{
    this->w = w;
    this->buttonID = buttonID;
}

void MyWork::run()
{
    for (int i = 0; i < 5; ++i)
    {
        QThread::sleep(1);
        w->addItem(QString::number(quintptr(QThread::currentThreadId()))
                + " 按钮" + QString::number(buttonID)
                + ": " +QString::number(i + 1));
    }
}
