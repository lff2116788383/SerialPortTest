#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QtSerialPort/QSerialPort>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QStringList getPortNameList();//获取所有可用的串口列表

private slots:
    void on_btnOpenCOM_clicked();
    void receiveInfo();

    void on_btnSendData_clicked();

private:
    Ui::MainWindow *ui;
    QSerialPort* m_serialPort; //串口类
    QStringList m_portNameList;
};
#endif // MAINWINDOW_H
