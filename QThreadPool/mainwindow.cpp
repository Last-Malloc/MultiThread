#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_1_clicked()
{
    MyWork* t = new MyWork(ui->listWidget, 1);
    QThreadPool::globalInstance()->start(t);
}

void MainWindow::on_pushButton_2_clicked()
{
    MyWork* t = new MyWork(ui->listWidget, 2);
    QThreadPool::globalInstance()->start(t);
}
