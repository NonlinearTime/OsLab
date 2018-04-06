#ifndef REFRESHDIALOG_H
#define REFRESHDIALOG_H

#include <QDialog>
#include <QDebug>
#include <QDateTime>
#include <QString>
#include <QProcess>


namespace Ui {
class RefreshDialog;
}

class RefreshDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RefreshDialog(QWidget *parent = 0);
    ~RefreshDialog();

private slots:
    void on_pushButton_clicked();
    void onButtonCloseClicked();
    void show_counter();
    void show_sum();
    void show_time();

private:
    Ui::RefreshDialog *ui;
    qint32 sum;
    qint32 count;
    QProcess *counter, *adder , *timer;
};

#endif // REFRESHDIALOG_H
