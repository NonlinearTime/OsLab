#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    lab1copy = new CopyDialog(this,QString("/home/haines/os/lab1"),QString("Lab1 : 文件拷贝"));
    lab2copy = new CopyDialog(this,QString("/home/haines/os/lab2"),QString("Lab2 : 系统调用"));
    lab1fresh = new RefreshDialog(this);
    resourcemanager = new ResourceManager(this);
    resourcemanager->setWindowTitle(QString("任务管理器 by Haines"));
    lab3module = new ModuleDialog(this);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete lab1copy;
    delete lab2copy;
    delete lab1fresh;
    delete resourcemanager;
}

void MainWindow::on_pushButton_clicked()
{
    lab1copy->show();
}

void MainWindow::on_pushButton_3_clicked()
{
    lab1fresh->show();
}

void MainWindow::on_pushButton_2_clicked()
{
    lab2copy->show();
}

void MainWindow::on_pushButton_5_clicked()
{
    resourcemanager->show();
}

void MainWindow::on_pushButton_4_clicked()
{
    lab3module->show();
}
