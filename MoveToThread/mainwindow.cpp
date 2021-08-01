#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //向列表添加 "main thread: <线程号>"
    ui->listWidget->addItem("main thread: " + QString::number(quintptr(QThread::currentThreadId())));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_1_clicked()
{
    QThread *th = new QThread();
    MyWork *t = new MyWork();
    t->moveToThread(th);
    //线程结束时 销毁MyWork对象
    connect(th, &QThread::finished, t, &QObject::deleteLater);
    //线程结束时 销毁QThread对象
    connect(th, &QThread::finished, th, &QObject::deleteLater);
    //执行完特定操作后，及时终止线程
    connect(t, &MyWork::threadFin, this, &MainWindow::onThreadFin);
    th->start();

    mutex.lock();
    connect(this, &MainWindow::operate, t, &MyWork::onReceivedData);
    emit operate(ui->listWidget, 1, th);
    disconnect(this, &MainWindow::operate, t, &MyWork::onReceivedData);
    mutex.unlock();
}

void MainWindow::on_pushButton_2_clicked()
{
    QThread *th = new QThread();
    MyWork *t = new MyWork();
    t->moveToThread(th);
    connect(th, &QThread::finished, t, &QObject::deleteLater);
    connect(th, &QThread::finished, th, &QObject::deleteLater);
    connect(t, &MyWork::threadFin, this, &MainWindow::onThreadFin);
    th->start();

    mutex.lock();
    connect(this, &MainWindow::operate, t, &MyWork::onReceivedData);
    emit operate(ui->listWidget, 2, th);
    disconnect(this, &MainWindow::operate, t, &MyWork::onReceivedData);
    mutex.unlock();
}

void MainWindow::onThreadFin(QThread *th)
{
    th->quit();
    th->wait();
}
