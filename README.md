[csdn文章地址](https://blog.csdn.net/qq_42283621/article/details/119298968)

[github仓库地址](https://github.com/Last-Malloc/MultiThread)

# 一. 前言

本文所要提到的主要内容：

1. 介绍Qt多线程的 两种 实现方式 ：继承于QThread；moveToThread（实现过程主要参考Qt 5.12.5官方文档）
2. 用Qt语言，实现目前将要用到的两种应用：临时新建新线程完成小任务；常驻线程，为主线程服务。
3. 不同的实现方法通过一个小需求来进行串起，第2章先说明小需求并使用C#实现便于理解。
4. **在写这篇文章时，发现了Qt自带线程池，所以最后介绍了QThreadPool的一些操作**

# 二. 需求及其C#多线程实现

## 1.  需求

一个列表控件，程序运行时首先向其中添加主线程号；两个按钮（按钮1和按钮2），点击按钮i时，开启一个新线程，向列表控件中插入5个item，内容为<线程号+“按钮i”+(1,2,...,5)>，然后线程结束并回收。

窗口效果图：

![img](https://img-blog.csdnimg.cn/20210718215916756.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQyMjgzNjIx,size_16,color_FFFFFF,t_70)

执行效果图：

![img](https://img-blog.csdnimg.cn/20210718221055530.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQyMjgzNjIx,size_16,color_FFFFFF,t_70)

## 2. C#实现

注意事项：

1. 代码放在github的C_Sharp子目录下

2. 在构造函数中 CheckForIllegalCrossThreadCalls = false; 取消非法的跨线程访问检查

   C#默认会进行非法的跨线程访问检查，即线程只能访问自己创造的控件，图中的listbox由主线程创造，所以默认子线程不可访问。

3. Thread.CurrentThread.ManagedThreadId.ToString()获取当前线程的线程号

4. ```c#
   //定义并创建线程对象，参数为子线程要执行的方法，改方法应该没参数或只具有一个object参数
   Thread t = new Thread(fun);
   // 设置新线程在后台执行
   t.IsBackground = true;
   // 新线程启动，并向其要执行的fun函数传递一个object类型参数param
   t.Start(param);
   //如果fun没有参数，可以写作 t.start();
   //fun方法只能具有一个object参数，并不是意味着其只能传递给子线程一个参数，可以将众多参数放到一个列表里赋值给一个object（装箱），然后参数传递进去再进行类型转换（拆箱）。
   ```



# 三. Qt实现

## 1. 官方API注意事项

在写这一部分时，我首先进行了在csdn上查阅了很多参考文章，但是很多人的说法都不太一致，这时，就必须查阅大法宝**官方API**。

根据官方API对QThread Class的描述，我所注意到的有：

1. Qt提供两种 多线程实现方式 ：继承于QThread（#1）；moveToThread（#2）

   *要实现实际功能的类这里成为Worker类，#1中为继承于QThread的类，#2中为继承于QObject并被moveThread的类*

2. **#1方法是过时的方法，官方不再推荐使用，因为其不是线程安全的**。根据官方文档，我认为有以下几点原因

   1） 只有重写的run函数代码是运行在子线程的，**子线程在run函数执行完就会结束**，而其**被触发的槽函数和被调用的公有方法（包括构造函数）还是在主线程中执行**。这也意味着，#1方法只能执行run函数，并没有单独的事件循环。**但是，可以在子线程中emit信号**。

   2） 若向子线程传参数，只能通过构造函数 或 调用其函数为其类变量赋值 或 直接对其类变量赋值，而这三种方法都是运行在主线程的。这意味着，主线程和子线程可能同时访问同一个类变量，所以应该注意同步问题。

   *这里说明一下，主线程和子线程的概念是相对的，例如在线程A中创建了线程B，那么A就是相对B的主线程，相反B是相对于A的子线程*

3. **#2方法是官方推荐的方法，其将整个Worker类移动到了新线程中，是线程安全的**。#2方法**通过信号合槽来保持 主线程 和 子线程间的通信，其槽函数运行在子线程中**

4. **从Qt 4.8开始，可以通过将finished()信号连接到QObject::deleteLater()来释放处于刚刚结束的线程中的对象**

5. 线程启动和停止时会发送void finished()和void started()信号，通过bool QThread::isFinished() const和bool QThread::isRunning() const可以主动查询线程状态。

6. 可以通过调用exit()或quit()来停止线程。极端情况下可以使用terminate()，但这是强制退出线程，可能造成内存泄露十分危险。

7. 使用wait()来阻塞调用线程，直到另一个线程完成执行(或直到指定的时间过去)。sleep()、msleep()和usleep()分别为全秒、毫秒和微秒的睡眠函数。

   **Qt是事件驱动的，所以最好不要使用wait()和sleep()函数，因为其都是堵塞执行的。wait()等待子线程结束然后进行逻辑处理 可以替换为 侦听finished()信号然后进行逻辑处理。sleep()函数阻塞等待时间然后进行逻辑处理 可以使用QTimer计时到某时间后再触发逻辑处理**

## 2. 继承于QThread方法的主要代码

```c++
//mywork.h
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

//mywork.cpp
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
```

```c++
//mainwindow.h
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_1_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
};

//mainwindow.cpp
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
```

## 3. moveToThread方法

### 主要代码①

1. 在构造函数中 定义线程对象和工作对象 将工作对象移入新线程 信号合槽 启动新线程

   在析构函数中 结束子线程（因信号合槽，线程结束时，自动销毁MyWork对象）

2. **因为只有一个线程对象，所以连续发射operate信号时，槽函数将排队执行**

3. 线程在窗口的析构函数中进行了退出。若想执行完某个函数时及时退出，可以在MyWork类定义信号在某函数结束前emit，在MainWindow类定义槽，在槽函数中使用 workerThread.quit(); workerThread.wait();

4. QThread workerThread为类成员变量，由MainWindow对象负责销毁；若是 指针类型成员变量 或 局部变量QThread *workerThread，那么在线程退出后，应该delete  workerThread，或者在创建时使用信号合槽

   connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);

```c++
//mywork.h
public slots:
    void onReceivedData(QListWidget *w, int buttonID);

//mywork.cpp
void MyWork::onReceivedData(QListWidget *w, int buttonID)
{
    for (int i = 0; i < 5; ++i)
    {
        QThread::sleep(1);
        w->addItem(QString::number(quintptr(QThread::currentThreadId()))
                + " 按钮" + QString::number(buttonID)
                + ": " +QString::number(i + 1));
    }
}

//mainwindow.h
signals:
    void operate(QListWidget *w, int buttonID);

private slots:
    void on_pushButton_1_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;

    //线程对象
    QThread workerThread;

//mainwindow.cpp
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
        
	//向列表添加 "main thread: <线程号>"
    ui->listWidget->addItem("main thread: " + QString::number(quintptr(QThread::currentThreadId())));
        
    //创建工作对象
    MyWork *t = new MyWork();
    //将工作对象移入新线程
    t->moveToThread(&workerThread);
    //线程结束时，自动销毁MyWork对象（重要）
    connect(&workerThread, &QThread::finished, t, &QObject::deleteLater);
    //信号合槽
    connect(this, &MainWindow::operate, t, &MyWork::onReceivedData);
    //开启新线程
    workerThread.start();
}

MainWindow::~MainWindow()
{
    //结束子线程
    workerThread.quit();
    //主线程等待子线程结束
    workerThread.wait();

    //（重要）定义的QThread不是指针，否则此处需要delelte workThread
    //或者在构造函数的位置添加connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);

    delete ui;
}

void MainWindow::on_pushButton_1_clicked()
{
    emit operate(ui->listWidget, 1);
}

void MainWindow::on_pushButton_2_clicked()
{
    emit operate(ui->listWidget, 2);
}
```

### 主要代码②

**为了可以两个线程同时执行，可以使用两个QThread(MyWork类和①相同)**

```c++
//mainwindow.h
signals:
    void operate1(QListWidget *w, int buttonID);
    void operate2(QListWidget *w, int buttonID);

private slots:
    void on_pushButton_1_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;

    //线程对象
    QThread workerThread1;
    QThread workerThread2;

//mainwindow.cpp
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //向列表添加 "main thread: <线程号>"
    ui->listWidget->addItem("main thread: " + QString::number(quintptr(QThread::currentThreadId())));

    //创建工作对象
    MyWork *t1 = new MyWork();
    //将工作对象移入新线程
    t1->moveToThread(&workerThread1);
    //线程结束时，自动销毁MyWork对象（重要）
    connect(&workerThread1, &QThread::finished, t1, &QObject::deleteLater);
    //信号合槽
    connect(this, &MainWindow::operate1, t1, &MyWork::onReceivedData);
    //开启新线程
    workerThread1.start();

    //创建工作对象
    MyWork *t2 = new MyWork();
    //将工作对象移入新线程
    t2->moveToThread(&workerThread2);
    //线程结束时，自动销毁MyWork对象（重要）
    connect(&workerThread2, &QThread::finished, t2, &QObject::deleteLater);
    //信号合槽
    connect(this, &MainWindow::operate2, t2, &MyWork::onReceivedData);
    //开启新线程
    workerThread2.start();
}

MainWindow::~MainWindow()
{
    //结束子线程
    workerThread1.quit();
    //主线程等待子线程结束
    workerThread1.wait();

    //结束子线程
    workerThread2.quit();
    //主线程等待子线程结束
    workerThread2.wait();

  	//（重要）定义的QThread不是指针，否则此处需要delelte workThread
    //或者在构造函数的位置添加connect(workerThread, &QThread::finished, workerThread, &QObject::deleteLater);

    delete ui;
}

void MainWindow::on_pushButton_1_clicked()
{
    emit operate1(ui->listWidget, 1);
}

void MainWindow::on_pushButton_2_clicked()
{
    emit operate2(ui->listWidget, 2);
}
```

### 主要代码③ 

若想在使用时即时创建线程，在执行完操作后，自行销毁，也可以向下面这样写

这里先connet再disconnect的目的：快速点击两个按钮，第2次的emit operate信号将触发两个对象（在两个按钮点击函数中分别创建的MyWork对象）的onReceivedData，而我们所预想的是无论怎么点击，都只触发在自己的代码块中定义的那个MyWork对象。所以在emit完信号后，及时的进行disconnet。

但是，这个及时的disconnet是不可保证的，所以可以加一个锁Mutex来保证在disconnet之前，不会有新的emit operate信号被发射出来。

```c++
//mywork.h
signals:
    void threadFin(QThread *th);

public slots:
    void onReceivedData(QListWidget *w, int buttonID, QThread *th);

//mywork.cpp
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

//mainwindow.h
signals:
    void operate(QListWidget *w, int buttonID, QThread *th);

private slots:
    void on_pushButton_1_clicked();

    void on_pushButton_2_clicked();

    void onThreadFin(QThread *th);

private:
    QMutex mutex;

//mywindow.cpp
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
```

## 4. 线程池

1） 线程池用于管理线程，用来减少大量线程被创建和销毁时时的时间损耗。类名为QThreadPool，Qt程序默认有一个全局的线程池，可以通过globalInstance()获取。

2） 线程池中存放的工作对象需要继承自QRunnable，并重写run函数（只有run函数是在新线程中执行）。该对象默认执行完run函数后会被自动删除，当然你也可以使用QRunnable::setAutoDelete()更改是否自动删除。

3）调用maxThreadCount()查询线程池的最大线程数，没有自行设置的话默认是当前正在工作的线程数。使用setMaxThreadCount()可以更改来限制线程数，不更改的话默认没有上限。	

4）线程池中的线程停止工作的话默认30s过期并销毁，可以使用setExpiryTimeout()来更改限制，设置负数则其中的线程永不过期。

可以看到，虽然也是只有run函数在新线程中执行（线程不安全）但是，非常的简单高效。

```c++
//mywork.h
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

//mywork.cpp
MyWork::MyWork(QListWidget *w, int buttonID)
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

//mainwindow.h
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_1_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
};

//mainwindow.cpp
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
    //创建线程工作对象
    //默认工作对象执行完工作会自行销毁并退出线程池
    MyWork* t = new MyWork(ui->listWidget, 1);
    QThreadPool::globalInstance()->start(t);
}

void MainWindow::on_pushButton_2_clicked()
{
    MyWork* t = new MyWork(ui->listWidget, 2);
    QThreadPool::globalInstance()->start(t);
}
```

