#ifndef COPYDIALOG_H
#define COPYDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QFile>
#include <QString>
#include <QProcess>
#include <QDebug>
#include <QMessageBox>


namespace Ui {
class CopyDialog;
}

class CopyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CopyDialog(QWidget *parent = 0, QString program = QString(""), QString title= QString("title"));
    ~CopyDialog();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void show_Result();

private:
    Ui::CopyDialog *ui;
    QString SourceFile;
    QString TargetFile;
    QString Program;
    QStringList ArgList;
    QProcess process;
};

#endif // COPYDIALOG_H
