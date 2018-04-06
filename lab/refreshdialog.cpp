#include "refreshdialog.h"
#include "ui_refreshdialog.h"
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <fcntl.h>

RefreshDialog::RefreshDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RefreshDialog)
{
    ui->setupUi(this);
    ui->label_date->setText(tr("xxxx.xx.xx"));
    ui->label_counter->setText(tr("0"));
    ui->label_adder->setText(tr("0"));
    counter = new QProcess(this);
    adder = new QProcess(this);
    timer = new QProcess(this);
    connect(counter,SIGNAL(readyReadStandardOutput()),this,SLOT(show_counter()));
    connect(adder,SIGNAL(readyRead()),this,SLOT(show_sum()));
    connect(timer,SIGNAL(readyRead()),this,SLOT(show_time()));
}

RefreshDialog::~RefreshDialog()
{
    delete ui;
    delete counter;
    delete adder;
    delete timer;
}

void RefreshDialog::on_pushButton_clicked()
{
//    pid_t counter,adder;
//    counter = fork();
//    sum = 0;
//    count = 0;

//    if (counter == 0) {
//        while (1) {
//            qDebug() << count << endl;
//            ui->label_counter->setText(QString("%1").arg(count));
//            if (++count == 10) count = 0;
//            sleep(1);
//        }
//    } else if (counter > 0) {
//        adder = fork();
//        if (adder == 0) {
//            for (int i = 0 ; i <= 1000 ; ++i) {
//                sum += i;
//                qDebug() << sum << endl;
//                ui->label_adder->setText(QString("%1").arg(sum));
//                sleep(1);
//            }
//            exit(0);
//        } else {
//            QDateTime current_date_time = QDateTime::currentDateTime();
//            QString current_date = current_date_time.toString("yyyy.MM.dd hh:mm:ss ddd");
//            while (1) {
//                current_date_time = QDateTime::currentDateTime();
//                current_date = current_date_time.toString("yyyy.MM.dd hh:mm:ss ddd");
//                qDebug() << current_date << endl;
//                ui->label_date->setText(current_date);
//                sleep(1);
//            }
//        }
//    }

    counter->start("/home/haines/os/lab1_1");
    adder->start("/home/haines/os/lab1_2");
    timer->start("/home/haines/os/lab1_3");
}

void RefreshDialog::show_counter() {
    QString count = QString("Counter: ").append(counter->readAllStandardOutput());
    qDebug() << "showResult_counter: " << endl
                << count;
    ui->label_counter->setText(count);
}

void RefreshDialog::show_sum() {
    QString summ = QString("Sum:").append(QString(adder->readAllStandardOutput()));
    qDebug() << "showResult_adder: " << endl
                << summ;
    ui->label_adder->setText(summ);
}

void RefreshDialog::show_time() {
    QString timm =  QString("Time: ").append(QString(timer->readAllStandardOutput()));
    qDebug() << "showResult_timer: " << endl
                << timm;
    ui->label_date->setText(timm);
}

void RefreshDialog::onButtonCloseClicked() {
    counter->kill();
    adder->kill();
    timer->kill();
    close();
}
