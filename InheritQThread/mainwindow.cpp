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
    //创建子线程工作对象
    MyWork *t = new MyWork(ui->listWidget, 1, this);
    //线程结束时（即fun执行结束后）自动地释放 对象资源
    connect(t, &MyWork::finished, t, &QObject::deleteLater);
    //启动子线程工作对象
    t->start();
}

void MainWindow::on_pushButton_2_clicked()
{
    //创建子线程工作对象
    MyWork *t = new MyWork(ui->listWidget, 2, this);
    //线程结束时（即fun执行结束后）自动地释放 对象资源
    connect(t, &MyWork::finished, t, &QObject::deleteLater);
    //启动子线程工作对象
    t->start();
}
