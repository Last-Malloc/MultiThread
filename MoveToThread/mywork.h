#ifndef MYWORK_H
#define MYWORK_H

#include <QObject>
#include <QListWidget>
#include <QThread>

class MyWork : public QObject
{
    Q_OBJECT
public:
    explicit MyWork(QObject *parent = nullptr);

signals:
    void threadFin(QThread *th);

public slots:
    void onReceivedData(QListWidget *w, int buttonID, QThread *th);
};

#endif // MYWORK_H
