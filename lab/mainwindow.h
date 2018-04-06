#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "copydialog.h"
#include "refreshdialog.h"
#include "resourcemanager.h"
#include "moduledialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_4_clicked();

private:
    Ui::MainWindow *ui;
    CopyDialog *lab1copy;
    CopyDialog *lab2copy;
    RefreshDialog *lab1fresh;
    ResourceManager *resourcemanager;
    ModuleDialog *lab3module;
};

#endif // MAINWINDOW_H
