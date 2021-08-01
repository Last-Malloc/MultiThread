#include "mywork.h"

MyWork::MyWork(QObject *parent) : QObject(parent)
{

}

void MyWork::onReceivedData(QListWidget *w, int buttonID, QThread *th)
{
    for (int i = 0; i < 5; ++i)
    {
        QThread::sleep(1);
        w->addItem(QString::number(quintptr(QThread::currentThreadId()))
                + " 按钮" + QString::number(buttonID)
                + ": " +QString::number(i + 1));
    }
    emit threadFin(th);
}
