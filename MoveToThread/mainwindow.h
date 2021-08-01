#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QMutex>
#include "mywork.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void operate(QListWidget *w, int buttonID, QThread *th);

private slots:
    void on_pushButton_1_clicked();

    void on_pushButton_2_clicked();

    void onThreadFin(QThread *th);

private:
    Ui::MainWindow *ui;

    QMutex mutex;
};
#endif // MAINWINDOW_H
