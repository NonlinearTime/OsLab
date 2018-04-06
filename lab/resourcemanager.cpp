#include "resourcemanager.h"
#include "ui_resourcemanager.h"
#include <pwd.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>

ResourceManager::ResourceManager(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ResourceManager)
{
    ui->setupUi(this);
    timer = new QTimer(this);
    timer->start(100);
    connect(timer,SIGNAL(timeout()),this,SLOT(update_content()));
    connect(ui->tabWidget,SIGNAL(currentChanged(int)), this, SLOT(show_tap_content(int)));
    slice0 = slice1 = times0 = times1 = 0;
}

ResourceManager::~ResourceManager()
{
    delete ui;
    delete timer;
}


/*update_process
 * update process information such:
 * PID,name,mem...
*/
void ResourceManager::update_process()
{
    QFile file;
    QDir dir("/proc");
    QStringList entryList = dir.entryList();
//    qDebug() << entryList;
    bool ok;
    QString filename, tmp;
    QString procName, procPri, procStat, procMem;
    int sleeping = 0, running = 0;
    QTreeWidgetItem *cur=ui->treeWidget->currentItem();//record current item
    QString curr="\0";
    if (cur!=NULL)
        curr=cur->text(0);

    ui->treeWidget->clear();
    for (int i =2 ; ; ++i) {
        int pid_proc = entryList[i].toInt(&ok);
        if (!ok) break;
        filename = QString("/proc/%1/stat").arg(pid_proc);
        file.setFileName(filename);
//        qDebug() << pid_proc;
        if (!file.open(QFile::ReadOnly)) {
            QMessageBox::warning(this,QString("Warning"),QString("Open file stat fail!"));
            return ;
        }
        tmp = file.readLine();
        file.close();
        if (tmp.length()==0) {
//            file.close();
            break;
        }
        int head = tmp.indexOf(QString("("));
        int tail = tmp.indexOf(QString(")"));
        procName = tmp.mid(head+1,tail - head - 1);
        procName.trimmed();
//        qDebug() << procName;
        procStat = tmp.section(" ", 2, 2);
        procPri = tmp.section(" ", 17, 17);
        procMem = tmp.section(" ", 22, 22);
        if(procStat!="S"&&procStat!="R"){
//            file.close();
            continue;
        }
        if(procStat=="S")
            sleeping++;
        if(procStat=="R")
            running++;
        QTreeWidgetItem *item= new QTreeWidgetItem(ui->treeWidget);
        QString pid_str = QString("%1").arg(pid_proc);
        while(pid_str.length()<5)
            pid_str = "0" + pid_str;
        while(procMem.length()<10)
            procMem = "0"+procMem;
        item->setText(0,pid_str);
        item->setText(1,procName);
        item->setText(2,procStat);
        item->setText(3,procPri);
        item->setText(4,procMem);


//        file.close();

    }

  ui->processnum->setText(QString::fromUtf8("    进程总数:")+QString::number(running+sleeping)+QString::fromUtf8("\t\t运行进程数:")+QString::number(running) +QString::fromUtf8("\t\t睡眠进程数:")+QString::number(sleeping));

    if (ui->treeWidget->findItems(curr,Qt::MatchExactly,0).count()!=0)
        ui->treeWidget->setCurrentItem(ui->treeWidget->findItems(curr,Qt::MatchExactly,0)[0]);//选中保存的item



}

/*update_sysInfo:
 * update system information such:
 * user name, machine name
 * CPU name, CPU type, CPU frequence
 * system version, kernel version, mem size, boot time, running time
*/
void ResourceManager::update_sysInfo()
{
    QFile file;
    QString tmp;
    int pos;

    //get machine name
    file.setFileName(QString("/etc/hostname"));
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this,QString("Warning"),QString("Open file hostname fail !"));
        return;
    }
    tmp = file.readLine();
    ui->machineLabel->setText(tmp);
    file.close();

    //get user name
    struct passwd *pwd;
    pwd = getpwuid(getuid());
    tmp = QString(pwd->pw_name);
    ui->usrLabel->setText(tmp);

    //get cpu information
    file.setFileName(QString("/proc/cpuinfo"));
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this,QString("Warning"),QString("Open file cpuinfo fail !"));
        return;
    }
    while (1) {
        tmp = file.readLine();
        pos = tmp.indexOf(QString("vendor_id"));
        if (pos != -1) {
            pos += 12;
            QString type_Str = tmp.mid(pos,tmp.length() - 12);
            ui->cpuTypeLabel->setText(type_Str);
            continue;
        }
        pos = tmp.indexOf(QString("model name"));
        if (pos != -1) {
            pos +=13;
            QString cpuname_str = tmp.mid(pos,tmp.length() - 13);
            ui->cpuLabel->setText(cpuname_str);
            continue;
        }
        pos = tmp.indexOf(QString("cpu MHz"));
        if (pos != -1) {
            pos += 11;
            QString cpuFre_str = tmp.mid(pos,tmp.length() - 11);
            ui->freLabel->setText(cpuFre_str);
            break;
        }
    }
    file.close();

    //get system information
    file.setFileName(QString("/proc/version"));
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this,QString("Warning"),QString("Open file version fail !"));
        return;
    }
    tmp = file.readLine();
    pos = tmp.indexOf(QString("Linux version"));
    ui->syskernLabel->setText(tmp.mid(pos,20));
    pos = tmp.indexOf(QString("Ubuntu 5.4.0"));
    ui->sysVerLabel->setText(tmp.mid(pos,29));
    file.close();

    //get memory information
    file.setFileName(QString("/proc/meminfo"));
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this,QString("Warning"),QString("Open file meminfo fail !"));
        return;
    }
    tmp = file.readLine();
    double size;
    QString mem = tmp.mid(16,8);
    size = mem.toInt();
    size = size / (1024 * 1024);
    ui->sysmemLabel->setText(QString("%1").arg(size));

    //get system time
    struct sysinfo info;
    time_t current_time = 0;
    time_t boot_time = 0;
    time_t run_time = 0;
    struct tm *ptm;
    if (sysinfo(&info)) {
        QMessageBox::warning(this,QString("Warning"),QString("Failed to get sysinfo!"));
        return ;
    }
    time(&current_time);
    if (current_time > info.uptime) boot_time = current_time - info.uptime;
    else boot_time = info.uptime - current_time;
    ptm = gmtime(&boot_time);

    QString boot_time_str =
            QString("%1-%2-%3 %4:%5:%6").arg(ptm->tm_year + 1900).arg(ptm->tm_mon + 1).arg(ptm->tm_mday).arg(ptm->tm_hour).arg(ptm->tm_min).arg(ptm->tm_sec);
    ui->sysBegLabel->setText(boot_time_str);

    run_time = info.uptime;
    ptm = gmtime(&run_time);
    QString run_time_str =
            QString("%1Y-%2M-%3D-%4h-%5m-%6s ").arg(ptm->tm_year - 70).arg(ptm->tm_mon).arg(ptm->tm_mday).arg(ptm->tm_hour).arg(ptm->tm_min).arg(ptm->tm_sec);
    ui->sysRunLabel->setText(run_time_str);
}

/*update_usage:
 * update resource usage:
 * cpu
 * memory
 * swap
*/
void ResourceManager::update_usage()
{
    QFile file;
    int pos;
    file.setFileName(QString("/proc/meminfo"));
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this,QString("Warning"),QString("Open file meminfo fail !"));
        return;
    }
    QString tmp, MemtTotal_str, MemFree_str, Buffers_str, Cached_str, SwapTotal_str, SwapFree_str;
    int MemTotal, MemFree, Buffers, Cached, SwapTotal, SwapFree;
    int Mem_usage, Swap_usage;
    while (1) {
        tmp = file.readLine();
        pos = tmp.indexOf(QString("MemTotal:"));
        if (pos != -1) {
            MemtTotal_str = tmp.mid(pos + 10,tmp.length() - 14);
//            qDebug() << MemtTotal_str;
            continue;
        }
        pos = tmp.indexOf(QString("MemFree:"));
        if (pos != -1) {
            MemFree_str = tmp.mid(pos + 9,tmp.length() - 13);
//            qDebug() << MemFree_str;
            continue;
        }
        pos = tmp.indexOf(QString("Buffers:"));
        if (pos != -1) {
            Buffers_str = tmp.mid(pos + 9,tmp.length() - 13);
//            qDebug() << Buffers_str;
            continue;
        }
        pos = tmp.indexOf(QString("Cached:"));
        if (pos != -1) {
            if (tmp.indexOf(QString("SwapCached:")) == -1) {
                Cached_str = tmp.mid(pos + 8,tmp.length() - 12);
//                qDebug() << Cached_str;
                continue;
            }
        }
        pos = tmp.indexOf(QString("SwapTotal:"));
        if (pos != -1) {
            SwapTotal_str = tmp.mid(pos + 11,tmp.length() - 15);
//            qDebug() << SwapTotal_str;
            continue;
        }
        pos = tmp.indexOf(QString("SwapFree:"));
        if (pos != -1) {
            SwapFree_str = tmp.mid(pos + 10,tmp.length() - 14);
//            qDebug() << SwapFree_str;
            break;
        }
    }
    MemTotal = MemtTotal_str.toInt();
    MemFree = MemFree_str.toInt();
    Buffers = Buffers_str.toInt();
    Cached = Cached_str.toInt();
    SwapTotal = SwapTotal_str.toInt();
    SwapFree = SwapFree_str.toInt();
//    qDebug() << SwapTotal;
//    qDebug() << SwapFree;
    ui->mem_total->setText(QString("%1 GB").arg((double)MemTotal / (1024 *1024)));
    ui->mem_used->setText(QString("%1 GB").arg((double)(MemTotal - MemFree - Buffers - Cached) / (1024 * 1024)));
    ui->mem_left->setText(QString("%1 GB").arg((double)(MemFree + Buffers + Cached) / (1024 * 1024)));
    ui->Swap_total->setText(QString("%1 GB").arg((double)SwapTotal / (1024 * 1024)));
    ui->Swap_used->setText(QString("%1 GB").arg((double)(SwapTotal - SwapFree) / (1024 * 1024)));
    ui->Swap_left->setText(QString("%1 GB").arg((double)SwapFree / (1024 *1024)));
    Mem_usage = 100 * (MemTotal - MemFree - Buffers - Cached) / MemTotal;
    Swap_usage = 100 * (SwapTotal - SwapFree);
//    qDebug() << Mem_usage;
    ui->mem_uasge->setValue(Mem_usage);
    ui->Swap_usage->setValue(Swap_usage);
    file.close();

    //cpu usage
    file.setFileName(QString("/proc/stat"));
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this,QString("Warning"),QString("Open file stat fail !"));
        return;
    }
    slice0 = slice1;
    times0 = times1;
    slice1 = times1 = 0;
    tmp = file.readLine();
    for(int i = 0 ; i < 7 ; ++i) {
        slice1 += tmp.section(" ", i+2, i+2).toInt();
        if (i == 3) {
            times1 += tmp.section(" ", i+2 , i+2).toInt();
        }
    }
    int m, n;
    m = slice1 - slice0;
    n = times1 - times0;
    ui->cpu_usage->setValue( (m - n)*100/n );
    file.close();


    file.setFileName(QString("/proc/cpuinfo"));
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this,QString("Warning"),QString("Open file cpuinfo fail !"));
        return;
    }
    while (1) {
        tmp = file.readLine();
        pos = tmp.indexOf(QString("cpu MHz"));
        if (pos != -1) {
            pos += 11;
            QString cpuFre_str = tmp.mid(pos,tmp.length() - 11);
            ui->cpu_frec->setText(cpuFre_str);
            break;
        }
    }
    ui->cpu_max->setText(QString("3.2 GHz"));
    file.close();
}

void ResourceManager::update_about()
{

}

void ResourceManager::show_tap_content(int index)
{
    if (index == 0) {
        timer->stop();
        timer->start(1000);
        update_process();
    } else if (index == 1) {
        timer->stop();
        timer->start(1000);
        update_usage();
    } else if (index == 2) {
        update_sysInfo();
    } else {
        update_about();
    }
}

void ResourceManager::update_content()
{
    int index = ui->tabWidget->currentIndex();
    if (index == 0) {
        timer->stop();
        timer->start(1000);
        update_process();
    } else if (index == 1) {
        timer->stop();
        timer->start(1000);
        update_usage();
    } else if (index == 2) {
        update_sysInfo();
    } else {
        update_about();
    }
}

void ResourceManager::on_shutdown_button_clicked()
{
    system("halt");
}

void ResourceManager::on_reboot_button_clicked()
{
    system("reboot");
}



void ResourceManager::on_killButton_clicked()
{
    QTreeWidgetItem *cur = ui->treeWidget->currentItem();
    QString pid_cur = cur->text(0);
    //获得进程号
//    QTreeWidgetItem *item = ui->listWidget_process->currentItem();
//    QString pro = item->text(0);
    pid_cur = pid_cur.section("\t", 0, 0);
    system("kill -9 " + pid_cur.toLatin1());
    update_process();
    if (ui->treeWidget->findItems(pid_cur,Qt::MatchExactly,0).count()==0)
        QMessageBox::warning(this, tr("kill"), QString::fromUtf8("该进程已被杀死!"), QMessageBox::Ok);
    else
        QMessageBox::warning(this, tr("kill"), QString::fromUtf8("该进程没有杀死!"), QMessageBox::Ok);
}

void ResourceManager::on_refreshButton_clicked()
{
    update_process();
}
