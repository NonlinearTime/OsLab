#include "moduledialog.h"
#include "ui_moduledialog.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

ModuleDialog::ModuleDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModuleDialog)
{
    ui->setupUi(this);
}

ModuleDialog::~ModuleDialog()
{
    delete ui;
}

void ModuleDialog::on_input_pushButton_clicked()
{
    int mydev, ret;
    /*open mydev*/
    mydev = ::open("/dev/mydev",O_RDWR | O_NONBLOCK);
    if (mydev < 0) {
        qDebug() << "open mydev fail !\n";
        return ;
    }
    qDebug() << "open mydev success !\n";
    QString in_text = ui->text_input->text();
    int len = in_text.length();
    ret = ::write(mydev,in_text.toLatin1().data(),len);
    if (ret < 0) qDebug("write mydev fail !\n");
    else {
        qDebug("write mydev success !\n");
        qDebug() << in_text.toLatin1().data();
        QMessageBox::warning(this,QString(""),QString("写入设备成功！"),QMessageBox::Yes);
    }
    ::close(mydev);
}

void ModuleDialog::on_output_pushButton_clicked()
{
    int mydev, ret;
    mydev = ::open("/dev/mydev",O_RDWR | O_NONBLOCK);

//    qDebug(buf);
    int len = ui->text_input->text().length();
    char buf[len+1];
    qDebug() << len;
    /*read mydev*/
    ret = ::read(mydev,buf,len);
    qDebug() << ret;

    if (ret < 0) qDebug("read my dev fail !\n");
    else {
        qDebug("read mydev success !\n");
        QMessageBox::warning(this,QString(""),QString("读取设备成功！"),QMessageBox::Yes);
    }
    buf[len] = '\0';
    qDebug() << buf ;
    ui->text_output->setText(QString(buf));
    ::close(mydev);
}
