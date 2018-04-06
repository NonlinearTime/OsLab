#include "copydialog.h"
#include "ui_copydialog.h"

CopyDialog::CopyDialog(QWidget *parent, QString program, QString title) :
    QDialog(parent),
    ui(new Ui::CopyDialog)
{
    ui->setupUi(this);
    Program = program;
    ui->label->setText(title);
//    connect(&process,SIGNAL(readyRead()),this,SLOT(show_Result()));
}

CopyDialog::~CopyDialog()
{
    delete ui;
}

void CopyDialog::on_pushButton_clicked()
{
    SourceFile = QFileDialog::getOpenFileName(this,tr("选择文件"));
//    ArgList.append(SourceFile);
}

void CopyDialog::on_pushButton_2_clicked()
{
    TargetFile = QFileDialog::getSaveFileName(this,tr("选择文件"));
//    ArgList.append(TargetFile);
}

void CopyDialog::on_pushButton_3_clicked()
{
    ArgList.clear();
    ArgList.append(SourceFile);
    ArgList.append(TargetFile);

    qDebug() << ArgList;
    process.start(Program,ArgList);
    if (process.waitForFinished() ) {
        QMessageBox::warning(this,tr("提示"),tr("拷贝完成！"));
    }

}

void CopyDialog::show_Result()
{
    qDebug() << "showResult: " << endl
                << QString(process.readAll());
}
