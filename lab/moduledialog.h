#ifndef MODULEDIALOG_H
#define MODULEDIALOG_H

#include <QDialog>
#include <QString>
#include <QFile>
#include <QMessageBox>
#include <QDebug>

namespace Ui {
class ModuleDialog;
}

class ModuleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ModuleDialog(QWidget *parent = 0);
    ~ModuleDialog();

private slots:
    void on_input_pushButton_clicked();

    void on_output_pushButton_clicked();

private:
    Ui::ModuleDialog *ui;
};

#endif // MODULEDIALOG_H
