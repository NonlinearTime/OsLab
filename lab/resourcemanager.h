#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QMainWindow>
#include <QWidget>
#include <QString>
#include <QLayout>
#include <QLabel>
#include <QTabWidget>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QDir>

namespace Ui {
class ResourceManager;
}

class ResourceManager : public QMainWindow
{
    Q_OBJECT

public:
    explicit ResourceManager(QWidget *parent = 0);
    ~ResourceManager();

private:
    Ui::ResourceManager *ui;
    void update_process();
    void update_sysInfo();
    void update_usage();
    void update_about();

    QTimer *timer;

    int slice0,slice1;
    int times0,times1;

private slots:
    void update_content();
    void show_tap_content(int index);

    void on_shutdown_button_clicked();
    void on_reboot_button_clicked();

    void on_killButton_clicked();
    void on_refreshButton_clicked();
};

#endif // RESOURCEMANAGER_H
