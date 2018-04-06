#include "mainwindow.h"
//#include "copydialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle(QString("操作系统课设展示程序 by Haines"));

    w.show();

    return a.exec();
}
